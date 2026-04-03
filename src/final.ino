#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

const char* ssid = "esp32esp32esp32";
const char* password = "esp32esp32esp32";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LED_AZUL 2

float myLat = 0.0;
float myLon = 0.0;

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 30000;

void piscarLed(int pin, int vezes) {
  for (int i = 0; i < vezes; i++) {
    digitalWrite(pin, HIGH);
    delay(300);
    digitalWrite(pin, LOW);
    delay(300);
  }
}

float distanceKm(float lat1, float lon1, float lat2, float lon2) {
  const float R = 6371.0;
  float dLat = radians(lat2 - lat1);
  float dLon = radians(lon2 - lon1);
  float a = sin(dLat/2)*sin(dLat/2) +
            cos(radians(lat1))*cos(radians(lat2))*
            sin(dLon/2)*sin(dLon/2);
  float c = 2 * atan2(sqrt(a), sqrt(1-a));
  return R * c;
}

float bearingTo(float lat1, float lon1, float lat2, float lon2) {
  float dLon = radians(lon2 - lon1);
  float y = sin(dLon) * cos(radians(lat2));
  float x = cos(radians(lat1)) * sin(radians(lat2)) -
            sin(radians(lat1)) * cos(radians(lat2)) * cos(dLon);
  return fmod((degrees(atan2(y, x)) + 360), 360);
}

void drawHeadingArrow(int x, int y, float heading) {
  float rad = radians(heading - 90);
  int len = 10;
  int x2 = x + cos(rad) * len;
  int y2 = y + sin(rad) * len;
  display.drawLine(x, y, x2, y2, SSD1306_WHITE);
  display.fillTriangle(x2, y2, x2 - 3, y2 + 5, x2 + 3, y2 + 5, SSD1306_WHITE);
}

void showLoadingBar() {
  int barWidth = 100, barHeight = 10;
  int barX = (SCREEN_WIDTH - barWidth) / 2;
  int barY = (SCREEN_HEIGHT - barHeight) / 2;
  const int totalSteps = 50;

  WiFi.begin(ssid, password);

  for (int currentStep = 0; currentStep <= totalSteps; currentStep++) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH - 80) / 2, barY - 15);
    display.print("A ligar WiFi...");
    display.drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
    int fillWidth = (barWidth - 2) * currentStep / totalSteps;
    if (fillWidth > 0) {
      display.fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, SSD1306_WHITE);
    }
    display.display();
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH - 80) / 2, barY - 15);
    display.print("WiFi ligado!");
    display.drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
    display.fillRect(barX + 1, barY + 1, barWidth - 2, barHeight - 2, SSD1306_WHITE);
    display.display();
    delay(500);
    piscarLed(LED_AZUL, 3);
    digitalWrite(LED_AZUL, HIGH);
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Falha ao conectar WiFi.");
    display.display();
    while (true) delay(1000);
  }
}

bool getMyLocation() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://ipinfo.io/json");
    int code = http.GET();
    if (code == 200) {
      String payload = http.getString();
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        const char* loc = doc["loc"]; // string no formato "lat,lon"
        if (loc != nullptr) {
          String locStr = String(loc);
          int commaIndex = locStr.indexOf(',');
          if (commaIndex != -1) {
            myLat = locStr.substring(0, commaIndex).toFloat();
            myLon = locStr.substring(commaIndex + 1).toFloat();
            http.end();
            return true;
          }
        }
      }
    }
    http.end();
  }
  return false;
}

bool getPlaneData() {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  float lamin = myLat - 0.5;
  float lamax = myLat + 0.5;
  float lomin = myLon - 0.5;
  float lomax = myLon + 0.5;

  String url = "https://opensky-network.org/api/states/all?lamin=" + String(lamin, 6) +
               "&lamax=" + String(lamax, 6) +
               "&lomin=" + String(lomin, 6) +
               "&lomax=" + String(lomax, 6);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(5000);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      http.end();
      return false;
    }

    JsonArray states = doc["states"];
    if (states.size() == 0) {
      http.end();
      return false;
    }

    float closestDist = 99999.0;
    JsonArray closest;

    for (JsonVariant v : states) {
      JsonArray a = v.as<JsonArray>();
      if (!a[6].isNull() && !a[5].isNull()) {
        float dist = distanceKm(myLat, myLon, a[6], a[5]);
        if (dist < closestDist) {
          closestDist = dist;
          closest = a;
        }
      }
    }

    if (closest.size() > 0) {
      display.clearDisplay();
      display.setTextSize(1);

      int y = 0;
      display.setCursor(0, y);
      display.print("ICAO: ");
      display.println(closest[0].as<const char*>());

      y += 8;
      display.setCursor(0, y);
      display.print(" CallSign: ");
      display.println(closest[1].as<const char*>());

      y += 8;
      display.setCursor(0, y);
      display.printf("Lat: %.4f", closest[6].as<float>());

      y += 8;
      display.setCursor(0, y);
      display.printf("Long: %.4f", closest[5].as<float>());

      y += 8;
      display.setCursor(0, y);
      display.printf("Vel: %.0f km/h", closest[9].as<float>() * 3.6);

      y += 8;
      display.setCursor(0, y);
      display.printf("Distancia: %.1f km", closestDist);

      y += 8;
      display.setCursor(0, y);
      display.print("Origem: ");
      display.println(closest[2].as<const char*>());

      // Desenhar seta no canto direito (ex: x=110, y=30)
      float heading = bearingTo(myLat, myLon, closest[6], closest[5]);
      drawHeadingArrow(110, 30, heading);

      display.display();
    }

    http.end();
    return true;
  }

  http.end();
  return false;
}

void showTimer() {
  int secondsLeft = (updateInterval - (millis() - lastUpdate)) / 1000;
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // sobrescreve fundo
  display.setCursor(SCREEN_WIDTH - 28, 0);
  display.printf("T:%02ds", secondsLeft);
  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_AZUL, OUTPUT);
  digitalWrite(LED_AZUL, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha OLED");
    while (true);
  }

  showLoadingBar();

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - 12 * 9) / 2, 20);
  display.println("Invictaeria");
  display.display();
  delay(1500);

  if (!getMyLocation()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Erro localizacao");
    display.display();
    while (true);
  }

  getPlaneData();
  lastUpdate = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;
    getPlaneData();
  }

  showTimer(); // mostra o temporizador sempre no canto
  delay(1000);
}
