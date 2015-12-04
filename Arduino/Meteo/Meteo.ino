#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
//#include <OneWire.h> // 2 lines down - DS18B20 not in use NOW
//#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "Logins.h" // contain ThingSpeak ApiKey, WiFi SSID and password

BH1750 lightMeter;  // 5 lines down - sensors beginning
Adafruit_BMP085 bmp;
DHT dht(D2, DHT22);
//OneWire oneWire(D3);
//DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // LCD beginning
WiFiClient client; // ESP8266 WiFi mode set to client

int lux,pressure,pm10i; // 2 lines down - variables for sensors values
float tempBmp,humDHT,tempDHT;
uint8_t DegreeBitmap[]= { 0x6, 0x9, 0x9, 0x6, 0x0, 0, 0, 0 }; // LCD degree char declaraion
const unsigned long intervalsend = 600000; // 2 lines down - Timer time initialization
unsigned long previousMillis = 0;

const char* pm10; // 5 lines down - variables to decode JSON
const char* indeks;
char buf[1000];
String decoded;
const char* host = "toyorg.alwaysdata.net";
int port = 80;

const char* server = "api.thingspeak.com";
const char* version = "Meteo 0.8.2";

void getjson(); // 2 lines down - need to this early function declaration to get code working
void decodeJson();

void setup() {
  Wire.begin(D5,D6); // 7 lines down - sensors initialization and LCD, installing degree char to LCD
  lcd.begin(16, 2);
  lcd.createChar(1, DegreeBitmap);
  lcd.home();
  dht.begin();
  lightMeter.begin();
  bmp.begin();

  WiFi.begin(ssid, password); // starting connectiong to WiFi

  lcd.print("Connecting ");
  lcd.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }
    lcd.setCursor(0, 1);
    lcd.print("Connected!");
    delay(500);

  MDNS.addService("http", "tcp", 80); // 2 lines down - staring MDNS and OTA service
  ArduinoOTA.begin();
  decodeJson(); // executing this function so won't get empty value
}

void backlight() { // function checking light amount and setting backlight on or off
  lux = lightMeter.readLightLevel();
  if (lux < 5) {
    lcd.noBacklight();
  } else {
    lcd.backlight();
  }
}

void readings() { // getting values from sensors
  lcd.home();
  tempBmp = bmp.readTemperature();
  pressure = bmp.readPressure()/100;
  tempDHT = dht.readTemperature();
  humDHT = dht.readHumidity();
  //sensors.setResolution(12);
  //sensors.requestTemperatures();
  //tempDS = sensors.getTempCByIndex(0);
  //tempDS = round(tempDS*10)/10.0;

  if (isnan(humDHT)) { // checking is DHT values are OK
    lcd.clear();
    lcd.home();
    lcd.print("DHT failed!");
    delay(1000);
    return;
  }

//  if (tempDS == 85 || tempDS == -127) { // checking is DS18B20 values are OK
//    lcd.clear();
//    lcd.home();
//    lcd.print("DS18B20 failed!");
//    delay(1000);
//    return;
//  }

}

double dewPoint(double celsius, double humidity) { // calculating dew point
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}

void Inside(int del) { // showing first "screen" on LCD, arg is time of delay() in ms
  lcd.clear(); // clearing LCD
  lcd.home(); // set upper-left position
  lcd.print("In: ");
  lcd.print(tempBmp, 1);
  lcd.print("\001C"); // use of degree char
  lcd.setCursor(0, 1); // set new line
  lcd.print("L: ");
  lcd.print(lux);
  lcd.print(" Lx");
  backlight();
  delay(del);
}

void Outside(int del) {
  lcd.clear();
  lcd.home();
  lcd.print("Out: ");
  lcd.print(tempDHT, 1);
  lcd.print("\001C");
  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(humDHT, 1);
  lcd.print(" %");
  backlight();
  delay(del);
}
void AirPre(int del) {
  lcd.clear();
  lcd.home();
  lcd.print("P: ");
  lcd.print(pressure);
  lcd.print(" hPa");
  lcd.setCursor(0, 1);
  lcd.print("A: ");
  lcd.print(indeks);
  lcd.print(" ");
  lcd.print(pm10i);
  backlight();
  delay(del);
}

void Debug(int del) {
  lcd.clear();
  lcd.home();
  lcd.print("Ver: ");
  lcd.print(version);
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  backlight();
  delay(del);
}

void DewandHeat(int del) {
  lcd.clear();
  lcd.home();
  lcd.print("DP: ");
  lcd.print(dewPoint(tempDHT, humDHT),1);
  lcd.print("\001C");
  lcd.setCursor(0, 1);
  lcd.print("HI: ");
  lcd.print(dht.computeHeatIndex(tempDHT, humDHT, false),1);
  lcd.print("\001C");
  backlight();
  delay(del);
}

void send() { // sending data to ThingSpeak
  lcd.clear();
  lcd.home();
  lcd.print("Sending!");
  if (client.connect(server,80)) {
      String postStr = apiKey;
             postStr +="&field1=";
             postStr += String(tempBmp);
             postStr +="&field2=";
             postStr += String(humDHT);
             postStr +="&field3=";
             postStr += String(tempDHT);
             postStr +="&field4=";
             postStr += String(lux);
             postStr +="&field5=";
             postStr += String(pressure);
             postStr +="&field6=";
             postStr += String(pm10i);

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

void getjson() { // getting JSON form host, port
  if (!client.connect(host, port)) {
    lcd.clear();
    lcd.home();
    lcd.print("Connection failed");
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
  } else {
    lcd.clear();
    lcd.home();
    lcd.print("Json failed");
  }
  decoded = line;
}

void decodeJson() { // decoding JSON
  getjson();
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(decoded);

  if (!root.success()) {
    lcd.clear();
    lcd.home();
    lcd.print("Parse failed");
  } else {
    pm10 = root["PM10"];
  }

    pm10i = atoi(pm10); // ASCII to int

    if (pm10i < 50) { // setting AQI based on pm10i value
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
 unsigned long currentMillis = millis(); // setting current time from boot

 ArduinoOTA.handle(); // starting OTA handler

 readings(); // 7 lines down - executing function showing data on LCD

 Inside(2000);
 Outside(2000);
 AirPre(2000);
 DewandHeat(2000);
 Debug(1000);

 if ((unsigned long)(currentMillis - previousMillis) >= intervalsend) { // checking statement conditions
   previousMillis = currentMillis; // save last time executing this
   decodeJson();
   send();
 }
}
