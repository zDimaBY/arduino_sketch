#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
//#include "FontsRus/CrystalNormal6.h"
#include <SPI.h>
#include <microDS18B20.h>

#define ONE_WIRE_BUS 2 // Data wire is plugged into port 2 on the Arduino
#define DS_SENSOR_AMOUNT 1 //кількість датчиків
//для иницилизации ТФТ
#define TFT_SCLK 13
#define TFT_MOSI 11
#define TFT_CS   10
#define TFT_RES  9
#define TFT_RS   8
#define TFT_BL_BACKLIGHT 3 //Регулыровка якркості

#define PIN_RELAY 5 //Курування реле машини
#define PIN_RELAY1 4

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_RS, TFT_RES);
MicroDS18B20<ONE_WIRE_BUS, DS_ADDR_MODE> sensor[DS_SENSOR_AMOUNT];

byte timerStarIntOled;
bool startEngine = true;
int TempD18B20;
unsigned long timerTFT, timeout, timerStar, timerLoop0;

float voltage; //Вольт метр
const float r1 = 154000; //опір резистора r1
const float r2 = 45000; // опір резистора r2

uint8_t addr[][8] = {
  {0x28, 0x37, 0x5C, 0x77, 0x91, 0xB, 0x2, 0xF0},
  //{0x28, 0xFF, 0x99, 0x80, 0x50, 0x17, 0x4, 0x4D},
  //{0x28, 0xFF, 0x53, 0xE5, 0x50, 0x17, 0x4, 0xC3},
};

void setup(void) {
  //Serial.begin(115200);
  pinMode(TFT_BL_BACKLIGHT, OUTPUT);
  analogWrite(TFT_BL_BACKLIGHT, 0); //подсветка ТФТ
  pinMode(PIN_RELAY, OUTPUT); // канал реле 1
  pinMode(PIN_RELAY1, OUTPUT); // канал реле 2
  digitalWrite(PIN_RELAY, HIGH);
  digitalWrite(PIN_RELAY1, LOW);
  for (int i = 0; i < DS_SENSOR_AMOUNT; i++) {
    sensor[i].setAddress(addr[i]);
  }
  //tft.setFont(&CrystalNormal6pt8b);
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.cp437(true);
  tft.fillScreen(ST77XX_BLACK);
  for (byte i = 0; i < 100; i++) {
    analogWrite(TFT_BL_BACKLIGHT, i);
    delay(10);
  }
  for (byte i = 0; i < 10; i++) {
    startTFTcar();
  }
  for (byte i = 100; i > 0; i--) {
    analogWrite(TFT_BL_BACKLIGHT, i);
    delay(10);
  }
  tft.fillScreen(ST77XX_BLACK);
  analogWrite(TFT_BL_BACKLIGHT, 30);
}

void loop() {
  if (millis() - timerLoop0 >= 1000) {
    timerLoop0 = millis();
    D18B20();
  }
}
void D18B20() {
  //Serial.println(TempD18B20);
  for (int i = 0; i < DS_SENSOR_AMOUNT; i++) {
    sensor[i].requestTemp();
  }
  TempD18B20 = sensor[0].getTemp();
  if (TempD18B20 >= 88) {
    digitalWrite(PIN_RELAY, LOW);
  }
  if (TempD18B20 <= 80) {
    digitalWrite(PIN_RELAY, HIGH);
  }
  relayStartEngine();//таймер выключение свечей
  TFTLSD();//Вывод температуры на дисплей
}
void TFTLSD() {
  uint32_t sec = millis() / 1000ul;
  int timeHours = (sec / 3600ul);
  int timeMins = (sec % 3600ul) / 60ul;
  int timeSecs = (sec % 3600ul) % 60ul;
  //tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(10, 0);
  tft.print(utf8rus2("Темп.Двиг."));
  tft.print(TempD18B20);
  tft.print("\xB0  ");
  if (digitalRead(PIN_RELAY) == 0) {
    tft.setCursor(10, 10);
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.print(utf8rus2("Вент.ON "));
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  } else {
    tft.setCursor(10, 10);
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    tft.print(utf8rus2("Вент.OFF"));
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  }
  if (digitalRead(PIN_RELAY1) == 1) {
    tft.setCursor(10, 20);
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    tft.print(utf8rus2("Свiчки:OFF"));
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  } else {
    timerStarIntOled = (timerStar - millis()) / 1000;
    tft.setCursor(10, 20);
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.print(utf8rus2("Свiчки:"));
    tft.print(timerStarIntOled);
    tft.print("   ");
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  }
  voltmeter();
  tft.setCursor(10, 140);
  tft.print(utf8rus2("активнiсть:"));
  tft.setTextColor(ST77XX_MAGENTA, ST77XX_BLACK);
  tft.setCursor(10, 150);
  tft.print(timeHours);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print("h ");
  tft.setTextColor(ST77XX_MAGENTA, ST77XX_BLACK);
  tft.print(timeMins);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print("m ");
  tft.setTextColor(ST77XX_MAGENTA, ST77XX_BLACK);
  tft.print(timeSecs);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print("s  ");
}
void relayStartEngine() {
  if (startEngine) {
    if (TempD18B20 <= -25) {
      timerStar = 90000;
      heatedCandles();
    } else if (TempD18B20 <= -10) {
      timerStar = 75000;
      heatedCandles();
    } else if (TempD18B20 <= 0) {
      timerStar = 60000;
      heatedCandles();
    } else if (TempD18B20 <= 10) {
      timerStar = 30000;
      heatedCandles();
    } else if (TempD18B20 <= 40) {
      timerStar = 15000;
      heatedCandles();
    } else if (TempD18B20 <= 60) {
      timerStar = 10000;
      heatedCandles();
    } else {
      timerStar = 50;
      heatedCandles();
    }
  }
}
void heatedCandles() {
  if (millis() - timeout >= timerStar ) {
    digitalWrite(PIN_RELAY1, HIGH);
    startEngine = false;
    timeout = millis();
    return;
  }
}
void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}
void voltmeter() {
  float voltage = (analogRead(A0) * 5.0) / 1024 / (r2 / (r1 + r2));
  tft.setCursor(10, 30);
  if (10 > voltage) {
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
  } else {
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  }
  tft.print(utf8rus2("АКБ:"));
  tft.print(voltage, 2);
  tft.print("   ");
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
}
void startTFTcar() {
  tft.setCursor(40, 55);
  tft.print("MAZDA");
  tft.setCursor(50, 65);
  tft.print("626 GD");
  tft.setCursor(37, 80);
  tft.print(utf8rus2("запалення"));
  tft.setCursor(37, 90);
  tft.print(utf8rus2("увiмкнуте"));
  uint16_t color = 100;
  int i;
  int t;
  for (t = 0 ; t <= 4; t += 1) {
    int x = 0;
    int y = 0;
    int w = tft.width();
    int h = tft.height();
    for (i = 0 ; i <= 16; i += 1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x += 2;
      y += 3;
      w -= 4;
      h -= 6;
      color += 1900;
    }
    color += 100;
  }
}
