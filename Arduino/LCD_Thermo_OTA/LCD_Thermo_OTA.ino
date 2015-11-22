#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

BH1750 lightMeter;
Adafruit_BMP085 bmp;
DHT dht(D2, DHT22);
OneWire oneWire(D3);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
WiFiClient client;

int lux,pressure;
float tempBmp,humidity,tempOut;
uint8_t DegreeBitmap[]= { 0x6, 0x9, 0x9, 0x6, 0x0, 0, 0, 0 };
unsigned long intervalsend = 600000;
unsigned long previous = 0;

const char* pm10;
const char* indeks;
char buf[1000];
String decoded;
const char* host = "toyorg.alwaysdata.net";
int port = 80;

String apiKey = "APIKEY";
const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* server = "api.thingspeak.com";
const char* version = "OTA-0.6";
void getjson();
void show();

void setup() {
  Serial.begin(115200);
  Wire.begin(D5,D6);
  lcd.begin(16, 2);
  lcd.createChar (1, DegreeBitmap);
  lcd.home();
  dht.begin();
  lightMeter.begin();
  bmp.begin();
  WiFi.begin(ssid, password);

  lcd.print("Connecting ");
  lcd.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }
    lcd.setCursor(0, 1);
    lcd.print("Connected!");
    delay(500);

  MDNS.addService("http", "tcp", 80);
  ArduinoOTA.begin();
  show();
}

void backlight() {
  lux = lightMeter.readLightLevel();
  if (lux < 5) {
    lcd.noBacklight();
  } else {
    lcd.backlight();
  }
}

void readings() {
  lcd.home();
  tempBmp = bmp.readTemperature();
  pressure = bmp.readPressure()/100;
  humidity = dht.readHumidity();
  sensors.setResolution(12);
  sensors.requestTemperatures();
  tempOut = sensors.getTempCByIndex(0);
  tempOut = round(tempOut*10)/10.0;

  if (isnan(humidity)) {
    lcd.println("DHT failed!");
    delay(1000);
    return;
  }

  if (tempOut == 85 || tempOut == -127) {
    lcd.println("DS18B20 failed!");
    delay(1000);
    return;
  }
  yield();
}

void first() {
  lcd.clear();
  lcd.home();
  lcd.print("In: ");
  lcd.print(tempBmp, 1);
  lcd.print("\001C");
  lcd.setCursor(0, 1);
  lcd.print("Out: ");
  lcd.print(tempOut, 1);
  lcd.print("\001C");
  backlight();
  delay(2000);
  yield();
}

void second() {
  lcd.clear();
  lcd.home();
  lcd.print("Hum: ");
  lcd.print(humidity, 1);
  lcd.print(" %");
  lcd.setCursor(0, 1);
  lcd.print("Pre: ");
  lcd.print(pressure);
  lcd.print(" hPa");
  backlight();
  delay(2000);
  yield();
}

void third() {
  lcd.clear();
  lcd.home();
  lcd.print("L: ");
  lcd.print(lux);
  lcd.print(" Lx");
  lcd.setCursor(0, 1);
  lcd.print("Air: ");
  lcd.print(indeks);
  backlight();
  delay(2000);
  yield();
}

void fourth() {
  lcd.clear();
  lcd.home();
  lcd.print("Ver: ");
  lcd.print(version);
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  backlight();
  delay(1000);
  yield();
}

void send() {
  lcd.home();
  lcd.print("Sending!");
  if (client.connect(server,80)) {
      String postStr = apiKey;
             postStr +="&field1=";
             postStr += String(tempBmp);
             postStr +="&field2=";
             postStr += String(humidity);
             postStr +="&field3=";
             postStr += String(tempOut);
             postStr +="&field4=";
             postStr += String(lux);
             postStr +="&field5=";
             postStr += String(pressure);

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
       lcd.setCursor(0, 1);
       lcd.print("Sent!");
    }
 }

void getjson() {
  if (!client.connect(host, port)) {
    //Serial.println("connection failed");
    return;
  }

  String url = "/values.json";

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
  //  Serial.println("Ok");
  } else {
    //Serial.println("Failed");
  }
  decoded = line;
}

void show() {
  getjson();
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(decoded);

  if (!root.success()) {
    //Serial.println("parseObject() failed");
  } else {
    pm10 = root["PM10"];
  }

    int pm10i = atoi(pm10);

    if (pm10i < 50) {
      indeks = "Good";
    } else if (pm10i > 50 && pm10i < 100) {
      indeks = "Moderate";
    } else if (pm10i > 100 && pm10i < 150) {
      indeks = "Unhealthy SG";
    } else if (pm10i > 150 && pm10i < 200) {
      indeks = "Unhealthy";
    } else if (pm10i > 200 && pm10i < 300) {
      indeks = "V Unhealthy";
    } else if (pm10i > 300) {
      indeks = "Hazardous";
    }
}

void loop() {
 unsigned long current = millis();
 
 ArduinoOTA.handle();

 readings();

 first();
 second();
 third();
 fourth();

 if ((unsigned long)(current - previous) >= intervalsend) {
   send();
   show();
   previous = current;
 }
}