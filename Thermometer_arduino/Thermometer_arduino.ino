#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>

String apiKey = "Thingspeak APIkey";
const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* server = "api.thingspeak.com";

DHT dht(D2, DHT22);
OneWire oneWire(D3);
DallasTemperature sensors(&oneWire);
WiFiClient client;

void setup() {
  Serial.begin(9600);
  delay(10);
  dht.begin();
  sensors.begin();

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
      Serial.println("");
      Serial.println("WiFi connected");
  }

void loop() {
  float humidity = dht.readHumidity();
  float tempIn = dht.readTemperature();
  sensors.requestTemperatures();
  float tempOut = sensors.getTempCByIndex(0);

  if (isnan(humidity) || isnan(tempIn)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    return;
  }

  if (tempOut == 85 || tempOut == -127) {
    Serial.println("Failed to read from DS18B20 sensor!");
    delay(1000);
    return;
  }

  if (client.connect(server,80)) {
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(tempIn);
           postStr +="&field2=";
           postStr += String(humidity);
           postStr +="&field3=";
           postStr += String(tempOut);

     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(postStr.length());
     client.print("\n\n");
     client.print(postStr);
     client.stop();
     Serial.println("Sent!\n--------\nDeep Sleep Mode!");
     ESP.deepSleep(600000000);
    }
}
