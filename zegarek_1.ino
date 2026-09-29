#include <WiFi.h>
#include <Wire.h>
#include <time.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =========================
// WIFI
// =========================

const char* WIFI_SSID = "PLAY_Swiatlowodowy_5BCC";
const char* WIFI_PASSWORD = "yjKmJqJ2Yu";

// =========================
// OLED
// =========================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =========================
// POLSKA STREFA CZASOWA
// CET / CEST
// =========================

const char* TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

// Nazwy dni tygodnia
const char* dniTygodnia[] = {
  "Niedziela",
  "Poniedzialek",
  "Wtorek",
  "Sroda",
  "Czwartek",
  "Piatek",
  "Sobota"
};


// Funkcja do centrowania tekstu
void printCentered(String text, int y, int size) {
  display.setTextSize(size);

  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(
    text,
    0,
    y,
    &x1,
    &y1,
    &w,
    &h
  );

  int x = (SCREEN_WIDTH - w) / 2;

  display.setCursor(x, y);
  display.print(text);
}


// =========================
// SETUP
// =========================

void setup() {

  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Blad OLED!");
    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  printCentered("Laczenie z WiFi", 20, 1);
  display.display();


  // =========================
  // WIFI
  // =========================

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi polaczone!");

  display.clearDisplay();
  printCentered("WiFi OK", 15, 2);
  printCentered("Pobieram czas...", 40, 1);
  display.display();


  // =========================
  // NTP
  // =========================

  configTzTime(
    TZ_INFO,
    "pool.ntp.org",
    "time.google.com",
    "time.cloudflare.com"
  );


  // Poczekaj aż pobierzemy prawidłowy czas

  struct tm timeinfo;

  while (!getLocalTime(&timeinfo)) {
    Serial.println("Czekam na NTP...");
    delay(500);
  }

  Serial.println("Czas zsynchronizowany!");

  // Po synchronizacji WiFi nie jest już potrzebne.
  // ESP32 ma wewnętrzny zegar RTC.

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}


// =========================
// LOOP
// =========================

void loop() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Blad odczytu czasu");
    delay(1000);
    return;
  }


  // =========================
  // GODZINA
  // =========================

  char godzina[6];

  snprintf(
    godzina,
    sizeof(godzina),
    "%02d:%02d",
    timeinfo.tm_hour,
    timeinfo.tm_min
  );


  // =========================
  // SEKUNDY
  // =========================

  char sekundy[3];

  snprintf(
    sekundy,
    sizeof(sekundy),
    "%02d",
    timeinfo.tm_sec
  );


  // =========================
  // DATA
  // =========================

  char data[11];

  snprintf(
    data,
    sizeof(data),
    "%02d.%02d.%04d",
    timeinfo.tm_mday,
    timeinfo.tm_mon + 1,
    timeinfo.tm_year + 1900
  );


  // =========================
  // RYSOWANIE
  // =========================

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);


  // Dzien tygodnia

  printCentered(
    dniTygodnia[timeinfo.tm_wday],
    0,
    1
  );


  // Duza godzina

  printCentered(
    String(godzina),
    14,
    3
  );


  // Sekundy po prawej

  display.setTextSize(1);
  display.setCursor(110, 39);
  display.print(sekundy);


  // Data

  printCentered(
    String(data),
    54,
    1
  );


  display.display();

  delay(250);
}