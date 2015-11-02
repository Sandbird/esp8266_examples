#include <ESP8266WiFi.h>

String apiKey = "Thingspeak APIkey";
const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* server = "api.thingspeak.com";

WiFiClient client;
ADC_MODE(ADC_VCC);

void setup() {
  delay(10);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    }
  }

void loop() {
  int voltage = ESP.getVcc();
  if (client.connect(server,80)) {
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(voltage);

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

     ESP.deepSleep(600000000);
    }
}