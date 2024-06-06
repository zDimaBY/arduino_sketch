#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h> // Бібліотека для OTA-прошивки

#define SERVO_PIN 5
#define SERVO1_PIN 4

const char *ssid = "*****"; // Ім'я мережі WiFi
const char *password = "******"; // Пароль WiFi

Servo myservo;
Servo myservo1;

ESP8266WebServer server(80);

String header;

String valueString = "80"; // Стандартне значення вертикалі
String valueString1 = "15"; // Стандартне значення горизонталі
int pos1 = 0;
int pos2 = 0;

unsigned long previousMillis = 0;
const long interval = 30000; // Інтервал у мілісекундах вимкнення вервоприводів у стані спокою

void setup() {
  Serial.begin(9600);
  Serial.print("Підключаємось до ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  ArduinoOTA.setHostname("ESP8266_Servo_Antenna");
  ArduinoOTA.begin();

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  if (millis() - previousMillis >= interval) {
    myservo.detach();
    myservo1.detach();
  }
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
  html += "<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; }</style>";
  html += "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>";
  html += "</head><body><h1>ESP8266 Управління сервомотором антени</h1>";
  html += "<p>Позиція верх-вниз: <span id=\"servoPos\"></span></p>";
  html += "<input type=\"range\" min=\"65\" max=\"85\" class=\"slider\" id=\"servoSlider\" value=\"" + valueString + "\"/>";
  html += "<p>Позиція ліво-право: <span id=\"servoPos1\"></span></p>";
  html += "<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoSlider1\" value=\"" + valueString1 + "\"/>";
  html += "<script>var slider = document.getElementById(\"servoSlider\");";
  html += "var slider1 = document.getElementById(\"servoSlider1\");";
  html += "var servoP = document.getElementById(\"servoPos\"); servoP.innerHTML = slider.value;";
  html += "var servoP1 = document.getElementById(\"servoPos1\"); servoP1.innerHTML = slider1.value;";
  html += "slider.oninput = function() { servoP.innerHTML = this.value; $.get(\"/?value=\" + this.value); };";
  html += "slider1.oninput = function() { servoP1.innerHTML = this.value; $.get(\"/?value1=\" + this.value); };";
  html += "</script></body></html>";

  server.send(200, "text/html", html);

  if (server.hasArg("value")) {
    valueString = server.arg("value");
    myservo.attach(SERVO_PIN, 544, 2700);
    myservo.write(valueString.toInt());
    Serial.print("Got servo val: ");
    Serial.println(valueString);
    previousMillis = millis();
  }

  if (server.hasArg("value1")) {
    valueString1 = server.arg("value1");
    myservo1.attach(SERVO1_PIN, 544, 2700);
    myservo1.write(valueString1.toInt());
    Serial.print("Got servo1 val: ");
    Serial.println(valueString1);
    previousMillis = millis();
  }
}
