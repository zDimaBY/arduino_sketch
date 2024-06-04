#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <GyverNTP.h>

#define BATPIN A0
#define PinOutRelay0 16

float volt;
float r1 = 39300.0; // Resistor value R1
float r2 = 10000.0; // Resistor value R2

GyverNTP ntp(3);
ESP8266WebServer server(80);

const uint16_t lengt = 48; // Max trend points
uint16_t tick = 0;
float voltag[lengt];
String DataString[lengt];

byte statusNTP;
int currentHour = -1; // Track the current hour
unsigned long one = 0;

void setup(void) {
  Serial.begin(115200);
  pinMode(PinOutRelay0, OUTPUT);
  digitalWrite(PinOutRelay0, LOW);
  
  WiFi.begin("Aleksander", "11406080");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  ntp.begin();
  statusNTP = ntp.updateNow();
  
  server.on("/", handleRoot);
  server.begin();
  
  ArduinoOTA.setHostname("esp8266_relay_webServer_voltmetr");
  ArduinoOTA.begin();
  
  voltage();
}

void loop(void) {
  server.handleClient();
  ntp.tick();
  ArduinoOTA.handle();
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - one >= 60000) { // 1 minute
    one = currentMillis;
    voltage();
    
    // Check if the hour has changed
    if (statusNTP == 0 && ntp.hour() != currentHour) {
      currentHour = ntp.hour();
      tochka();
    }
  }
}

void tochka() {
  if (statusNTP != 0) {
    statusNTP = ntp.updateNow();
    Serial.println(statusNTP);
    return;
  }

  voltag[tick] = volt;
  DataString[tick] = String(ntp.year()) + "," + String(ntp.month() - 1) + "," + String(ntp.day()) + "," + String(ntp.hour()) + "," + String(ntp.minute()) + "," + String(ntp.second());
  
  Serial.print(ntp.timeString());
  Serial.print(" ");
  Serial.println(ntp.dateString());

  tick = (tick + 1) % lengt; // Move to the next index, loop around if necessary
}

void voltage() {
  float temp = (analogRead(BATPIN) * 5.0) / 1024.0;
  volt = temp / (r2 / (r1 + r2));
  
  Serial.print(volt);
  if (volt < 9.50) {
    digitalWrite(PinOutRelay0, LOW);
    Serial.println(" OFF");
  } else if (volt > 10.0) {
    digitalWrite(PinOutRelay0, HIGH);
    Serial.println(" ON");
  }
}

void handleRoot() {
  String trendstr = F(
    "<html>\
    <head>\
      <script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>\
      <meta http-equiv='refresh' content='1000'/>\
      <meta charset='UTF-8'>\
      <title>Інтернет БДЖ</title>\
      <script type='text/javascript'>\
        google.charts.load('current', {packages: ['corechart', 'line']});\
        google.charts.setOnLoadCallback(drawChart);\
        function drawChart() {\
          var data = new google.visualization.DataTable();\
          data.addColumn('datetime', 'Time');\
          data.addColumn('number', 'Вольт АКБ');\
          data.addRows([");

  for (int i = 0; i < lengt; i++) {
    int index = (tick + i) % lengt;
    if (!DataString[index].isEmpty()) {
      trendstr += "[new Date(" + DataString[index] + ")," + String(voltag[index]) + "],";
    }
  }

  trendstr += F("]);\
          var options = {width: '100%', curveType: 'function', backgroundColor: '#000000', colors: ['red'], pointSize: 8, legend: { position: 'bottom' }, vAxis: {textStyle:{color: '#FFF'}}, legendTextStyle: { color: '#FFF' }, titleTextStyle: { color: '#FFF' }, hAxis: { format: 'dd.MM.yyyy HH:mm', textStyle:{color: '#FFF'}, gridlines: { count: 10, }}};\
          var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));\
          var formatter = new google.visualization.DateFormat({pattern: 'dd.MM.yyyy HH:mm'});\
          formatter.format(data, 0);\
          chart.draw(data, options);\
        }\
      </script>\
    </head>\
    <body>\
      <div id='curve_chart' style='width: 100%; height: 600px'></div>\
    </body>\
    <style>\
      body { background-color: black; }\
    </style>\
  </html>");
  
  server.send(200, F("text/html"), trendstr);
}
