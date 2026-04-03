#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// WiFi
const char* ssid = "esp32esp32esp32";
const char* password = "esp32esp32esp32";

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LEDs
#define LED_AZUL 2
#define LED_VERMELHO 4

// Localização
float myLat = 0.0;
float myLon = 0.0;

// Temporizador
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 30000;

float distanceKm(float lat1, float lon1, float lat2, float lon2) {
  const float R = 6371.0;
  float dLat = radians(lat2 - lat1);
  float dLon = radians(lon2 - lon1);
  float a = sin(dLat/2)*sin(dLat/2) + cos(radians(lat1))*cos(radians(lat2))*sin(dLon/2)*sin(dLon/2);
  float c = 2 * atan2(sqrt(a), sqrt(1-a));
  return R * c;
}

void piscarLed(int pin, int vezes) {
  for (int i = 0; i < vezes; i++) {
    digitalWrite(pin, HIGH);
    delay(300);
    digitalWrite(pin, LOW);
    delay(300);
  }
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
  int barWidth = 100;
  int barHeight = 10;
  int barX = (SCREEN_WIDTH - barWidth) / 2;
  int barY = (SCREEN_HEIGHT - barHeight) / 2;
  const int totalSteps = 50; // 50 passos * 100ms = 5 segundos

  WiFi.begin(ssid, password);

  for (int currentStep = 0; currentStep <= totalSteps; currentStep++) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH - 80) / 2, barY - 15);
    display.print("A ligar WiFi...");

    // Desenha o contorno da barra
    display.drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);

    // Preenche a barra proporcional ao progresso
    int fillWidth = (barWidth - 2) * currentStep / totalSteps;
    if (fillWidth > 0) {
      display.fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, SSD1306_WHITE);
    }

    display.display();

    // Continua tentando conectar enquanto a barra avança
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.status(); // atualiza status (normalmente não bloqueia)
    }

    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Mostra barra completa e pisca led azul
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
    // Se não conectou, mostra erro e para
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Falha ao conectar WiFi.");
    display.display();
    while (true) delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  digitalWrite(LED_AZUL, LOW);
  digitalWrite(LED_VERMELHO, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha OLED");
    while (true);
  }

  showLoadingBar();

  // Mostra texto do projeto por 1.5s
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

  // Primeira atualização dos dados do avião na inicialização
  if (!getPlaneData()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Sem dados de aviões");
    display.display();
  }

  lastUpdate = millis();
}

void loop() {
  unsigned long now = millis();

  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    if (!getPlaneData()) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Erro aviões");
      display.display();
    }
  } else {
    // Mostra contador regressivo limpo no canto superior direito
    int secondsLeft = (updateInterval - (now - lastUpdate)) / 1000;

    // Atualiza só a parte do contador para evitar "flicker"
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(SCREEN_WIDTH - 28, 0);
    display.print("T:");
    if (secondsLeft < 10) display.print("0");  // zerar alinhamento visual
    display.print(secondsLeft);
    display.display();
  }
}

bool getMyLocation() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://ip-api.com/json");
    int code = http.GET();
    if (code == 200) {
      String payload = http.getString();
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        myLat = doc["lat"];
        myLon = doc["lon"];
        http.end();
        return true;
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
  int code = http.GET();
  if (code != 200) {
    http.end();
    return false;
  }

  String payload = http.getString();
  StaticJsonDocument<8000> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    http.end();
    return false;
  }

  JsonArray states = doc["states"];
  if (states.isNull() || states.size() == 0) {
    http.end();
    return false;
  }

  float minDist = 99999;
  int idxClosest = -1;
  for (int i = 0; i < states.size(); i++) {
    float planeLat = states[i][6];
    float planeLon = states[i][5];
    if (planeLat == 0 && planeLon == 0) continue;
    float d = distanceKm(myLat, myLon, planeLat, planeLon);
    if (d < minDist) {
      minDist = d;
      idxClosest = i;
    }
  }

  if (idxClosest == -1) {
    http.end();
    return false;
  }

  JsonArray plane = states[idxClosest];
  String icao24 = plane[0].as<String>();
  String callsign = plane[1].as<String>();
  float velocity = plane[9];
  float heading = plane[10];
  float altitude = plane[13];
  float lat = plane[6];
  float lon = plane[5];
  bool onGround = plane[8];
  float velocityKmh = velocity * 3.6;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("ICAO24: "); display.println(icao24);
  display.print("Callsign: "); display.println(callsign.length() > 0 ? callsign : "N/A");
  display.print("Lat: "); display.println(lat, 4);
  display.print("Lon: "); display.println(lon, 4);
  display.print("Alt(m): "); display.println((int)altitude);
  display.print("Vel(km/h): "); display.println((int)velocityKmh);
  display.print("Dist(km): "); display.println(minDist, 2);
  display.print("Pousado: "); display.println(onGround ? "Sim" : "Nao");

  drawHeadingArrow(110, 55, heading);
  display.display();

  http.end();
  return true;
}
