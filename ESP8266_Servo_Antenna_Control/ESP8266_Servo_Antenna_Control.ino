//Зроблено zDimaBY для користувачів 4PDA Слава Україні !!
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h> // Бібліотека для OTA-прошивки

// GPIO-контакт, к которому подключен сервопривод:
#define servoPin 5
#define servo1Pin 4

const char *ssid = "Aleksander"; //Ім'я мережі вайфай
const char *password = "11406080"; //Пароль вайфай

Servo myservo;  //створюємо екземпляр класу «Servo»,
Servo myservo1; //щоб за його допомогою керувати сервоприводом. більшість плат дозволяють створити 12 об'єктів класу «Servo»

WiFiServer server(80);//створюємо веб-сервер на порті «80»:

String header;//змінна для зберігання HTTP-запиту:

String valueString = String(80);//кілька змінних для розшифрування значення в HTTP-запиті GET:
String valueString1 = String(100);
int pos1 = 0;
int pos2 = 0;

unsigned long one, two; //Таймери

void setup() {
  Serial.begin(9600);
  Serial.print("Підключаємось до ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //підключаємося до WiFi за допомогою зазначених вище SSID та пароля:
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  ArduinoOTA.setHostname("ESP8266_Серво_привід_антени"); //Задаємо ім'я мережного порту
  //ArduinoOTA.setPassword((const char *)"0000"); //Задаємо пароль доступу для віддаленої прошивки
  ArduinoOTA.begin(); // Инициализируем OTA
  
  Serial.println("");
  Serial.println("WiFi connected.");  //  "WiFi підключено."
  Serial.println("IP address: ");     //  "IP-адреса: "
  Serial.println(WiFi.localIP());//друкуємо локальну IP-адресу та запускаємо веб-сервер:
  server.begin();
}

void loop() {
  ArduinoOTA.handle(); // Завжди готові до прошивки
  WiFiClient client = server.available();// починаємо прослуховувати вхідних клієнтів:
  if (client) {                     // если подключился новый клиент,
    Serial.println("Новий клієнт.");  // друкуємо повідомлення
    String currentLine = "";        // створюємо рядок для зберігання
    // входящих данных от клиента;
    while (client.connected()) {    // цикл while() працюватиме
      // весь той час, поки клієнт буде підключений до сервера;
      if (client.available()) {     // якщо клієнт має дані, які можна прочитати,
        char c = client.read();     // зчитуємо байт, а потім
        Serial.write(c);            // друкуємо його в моніторі порту
        header += c;
        if (c == '\n') {            // якщо цим байтом є
          // символ нового рядка
           // якщо отримали два символи нового рядка поспіль,
           // це означає, що поточний рядок порожній;
           // це кінець HTTP-запиту клієнта,
           // отже – час відправляти відповідь:
          if (currentLine.length() == 0) {
            // HTTP-заголовки завжди починаються
             // з коду відповіді (наприклад, "HTTP/1.1 200 OK")
             // та інформації про тип контенту
             // (Щоб клієнт розумів, що отримує);
             // Наприкінці пишемо порожній рядок:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");//"З'єднання: вимкнено"
            client.println();
            // показуємо веб-сторінку:
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}");
            client.println(".slider { width: 300px; }</style>");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>"); //https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js https://192.168.8.1/lib/jquery-1.7.2.min.js
            // веб сторінка:
            client.println("</head><body><h1>Контроль сервомоторів антени через ESP8266</h1>");
            client.println("<p>Позиція верх-вниз: <span id=\"servoPos\"></span></p>");
            client.println("<input type=\"range\" min=\"65\" max=\"85\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\" value=\"" + valueString + "\"/>");
            client.println("<p>Позиція ліво-право: <span id=\"servoPos1\"></span></p>");
            client.println("<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoSlider1\" onchange=\"servo1(this.value)\" value=\"" + valueString1 + "\"/>");

            client.println("<script>var slider = document.getElementById(\"servoSlider\");");
            client.println("var slider1 = document.getElementById(\"servoSlider1\");");
            client.println("var servoP = document.getElementById(\"servoPos\"); servoP.innerHTML = slider.value;");
            client.println("var servoP1 = document.getElementById(\"servoPos1\"); servoP1.innerHTML = slider1.value;");
            client.println("slider.oninput = function() { slider.value = this.value; servoP.innerHTML = this.value; }");
            client.println("slider1.oninput = function() { slider1.value = this.value; servoP1.innerHTML = this.value; }");
            client.println("$.ajaxSetup({timeout:1000}); function servo(pos) { ");
            client.println("$.get(\"/?value=\" + pos + \"&\"); {Connection: close};}");
            client.println("function servo1(pos) { ");
            client.println("$.get(\"/?value1=\" + pos + \"&\"); {Connection: close};}</script>");

            client.println("</body></html>");

            //GET /?value=180& HTTP/1.1
            if (header.indexOf("GET /?value=") >= 0)
            {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1 + 1, pos2);

              //Обертайте сервопривід
              myservo.write(valueString.toInt());
              Serial.print("Got servro val: ");
              Serial.println(valueString);
            }//?value=
            else if (header.indexOf("GET /?value1=") >= 0)
            {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString1 = header.substring(pos1 + 1, pos2);

              //Обертайте сервопривід
              myservo1.write(valueString1.toInt());
              Serial.print("Got servro1 val: ");
              Serial.println(valueString1);
            }

            // кінець HTTP-відповіді задається
            // За допомогою додаткового порожнього рядка:
            client.println();
            // виходимо з циклу while():
            one = millis();
            myservo.attach(servoPin, 544, 2700);// прив'язуємо сервопривід,
            myservo1.attach(servo1Pin, 544, 2700);     // підключений до контакту "servoPin", до об'єкта "myservo"
            break;
          } else { // якщо отримали символ нового рядка,
            // Очищаємо поточний рядок "currentLine":
            currentLine = "";
          }
        } else if (c != '\r') {  // якщо отримали будь-які дані,
           // крім символу повернення каретки,
          currentLine += c;      // додаємо ці дані
           // наприкінці рядка «currentLine»
        }
      }
    }
    header = "";// очищаємо змінну «header[]»:
    client.stop();// відключаємо з'єднання
    Serial.println("Client disconnected.");//  "Клієнт відключився."
    Serial.println("");
  }
  if (millis() - one >= 30000) { //черех 30 секунд
    myservo.detach();// відключаємо сервоприводи
    myservo1.detach();
  }
}
