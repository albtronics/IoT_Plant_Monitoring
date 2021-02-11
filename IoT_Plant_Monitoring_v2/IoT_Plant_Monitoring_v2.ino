#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <BH1750.h>
#include <OneWire.h>
#include <SimpleTimer.h>
#include <ESP8266WiFi.h>
#include <DallasTemperature.h>
#include <BlynkSimpleEsp8266.h>

#define fanPin D0
#define dhtPin D1
#define relayPin D6
#define dhtType DHT11
#define moisturePin D5
#define ONE_WIRE_BUS D2
#define BLYNK_PRINT Serial

BH1750 lightMeter;
DHT dht(dhtPin, dhtType);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

char auth[] = "--------------------------------";              //Authentication code sent by Blynk
char ssid[] = "--------------------------------";              //WiFi SSID
char pass[] = "--------------------------------";

bool lastState = 0;
bool sensorState = 0;
byte humidityThreshold = 40;

SimpleTimer timer;

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V5, h);  //V5 is for Humidity
  Blynk.virtualWrite(V6, t);  //V6 is for Temperature

  if (h > humidityThreshold ) { // Change the value to shot simulation
    digitalWrite(fanPin, LOW);  // ON
  }
  if (h < humidityThreshold ) {
    digitalWrite(fanPin, HIGH); //OFF
  }
}

void setup() {
  dht.begin();
  sensors.begin();
  Wire.begin(D3, D4); //I2C Pins [ D3-SDA , D4-SCL ]
  lightMeter.begin();
  Serial.begin(115200);
  pinMode(fanPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(fanPin, HIGH);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendSensor);
}

byte i = 0;

void sendTemps()
{
  i = analogRead(A0);
  byte sensor = map(i,0,1024,0,100);
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  float lux = lightMeter.readLightLevel();
  Serial.println("Light: ");
  Serial.println(lux);
  Serial.println("Soil Temp: ");
  Serial.println(temp);
  Serial.println("Soil Moist: ");
  Serial.println(sensor);
  Blynk.virtualWrite(V1, lux);    // V1 - Light Intensity
  Blynk.virtualWrite(V2, temp);   // V2 - Soil Temperatue
  Blynk.virtualWrite(V3, sensor); // V3 - Soil Moisture
  delay(1000);
}

void loop() {
  Blynk.run();
  timer.run();
  sendTemps();
  sensorState = digitalRead(moisturePin);
  Serial.println(sensorState);

  if (sensorState == 1 && lastState == 0) {
    Serial.println("Needs Water, Send Notification");
    Blynk.notify("Water your Plants"); //Send Notification
    lastState = 1;
    delay(1000);
  }
  else if (sensorState == 1 && lastState == 1) {
    //Do nothing.
    Serial.println("Has not been Watered yet");
    delay(1000);
  }
  else {
    Serial.println("Does not need Water");
    lastState = 0;
    delay(1000);
  }
  delay(100);
}
