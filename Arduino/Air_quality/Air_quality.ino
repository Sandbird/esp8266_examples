// Stupid and unreadable Air Quality web server on ESP8266

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "ssid";
const char* password = "password";

const char* co;
const char* pm10;
const char* so2;
const char* bzn;
const char* o3;
const char* no2;
const char* indeks;

char buf[1000];
String decoded;

const char* host = "toyorg.alwaysdata.net";
int port = 80;

ESP8266WebServer server(80);

ADC_MODE(ADC_VCC);

void setup(void) {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();

  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}

void getjson() {
WiFiClient client;
Serial.print("connecting to ");
Serial.println(host);
if (!client.connect(host, port)) {
  Serial.println("connection failed");
  return;
}

String url = "/values.json";
Serial.println(url);

client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "User-Agent: BuildFailureDetectorESP8266\r\n" +
             "Connection: close\r\n\r\n");

while (client.connected()) {
  String line = client.readStringUntil('\n');
  if (line == "\r") {
    break;
  }
}
String line = client.readStringUntil('\n');
if (line.startsWith("{\"CO\"")) {
  Serial.println("Ok");
} else {
  Serial.println("Failed");
}
decoded = line;
}

void show() {
  getjson();
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(decoded);

  if (!root.success()) {
    Serial.println("parseObject() failed");
  } else {
    co = root["CO"];
    pm10 = root["PM10"];
    so2 = root["SO2"];
    bzn = root["BZN"];
    o3 = root["O3"];
    no2 = root["NO2"];
  }

    int pm10i = atoi(pm10);

    if (pm10i < 21) {
      indeks = "Bardzo dobra (5/5)";
    } else if (pm10i > 21 && pm10i < 81) {
      indeks = "Dobra (4/5)";
    } else if (pm10i > 81 && pm10i < 101) {
      indeks = "Zadowalająca (3/5)";
    } else if (pm10i > 101 && pm10i < 161) {
      indeks = "Dostateczna (2/5)";
    } else if (pm10i > 161 && pm10i < 280) {
      indeks = "Zła (1/5)";
    } else if (pm10i > 280) {
      indeks = "Bardzo zła (0/5)";
    }
}

void p(const char *fmt, ... ){
  va_list args;
  va_start (args, fmt);
#ifdef __AVR__
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
#else
  vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // for the rest of the world
#endif
  va_end(args);
}

void handleRoot() {
  int hr = (((millis() / 1000) / 60) / 60);
  int min = ((millis() / 1000) / 60) % 60;
  int sec = (millis() / 1000) % 60;

  show();

  int voltage = ESP.getVcc();

    p(
      "<html>\
      <head>\
      <title>ESP8266</title>\
      <meta charset=utf-8>\
      <link rel=stylesheet href=http://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/css/bootstrap.min.css >\
      </head>\
      <body>\
      <nav class='navbar navbar-default'>\
      <div class=container-fluid>\
      <div class=navbar-header>\
      <a class=navbar-brand>ESPThing</a>\
      </div>\
      <div>\
      </nav>\
      <div class=container-fluid>\
      <p>CO₂: %02s µg/m³</p>\
      <p>PM10: %02s µg/m³</p>\
      <p>SO₂: %02s µg/m³</p>\
      <p>BZN: %02s µg/m³</p>\
      <p>O₃: %02s µg/m³</p>\
      <p>NO₂: %02s µg/m³</p>\
      <p>Jakość powietrza: %02s</p>\
      <p>Napięcie: %02d</p>\
      <p>Uptime: %02d:%02d:%02d</p>\
      </div>\
      </body>\
      </html>",
                co, pm10, so2, bzn, o3, no2, indeks, voltage, hr, min, sec);

  server.send (1000, "text/html", buf);
}
