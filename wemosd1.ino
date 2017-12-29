
/* DHT Shield - Deep Sleep
 *
 * Displays humidity, temperature and heat index, sleeps for 10 seconds and repeats
 *
 * Connections:
 * D0 -- RST
 *
 * If you cant reprogram as the ESP is sleeping, disconnect D0 - RST and try again
 *
 * Depends on Adafruit DHT Arduino library
 * https://github.com/adafruit/DHT-sensor-library
 */

#include "DHT.h"
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#include <ESP8266WiFi.h>
String apiKey = "xxxxxxxxxxxxxxx";  
const char* ssid = "TheBananaStand";
const char* password = "xxxxxxxxxxxxxxx";
const char* server = "api.thingspeak.com";

// sleep for this many seconds
const int sleepSeconds = 15*60;

float humidity, temperature, heatIndex;
char str_humidity[10], str_temperature[10], str_heatIndex[10];
WiFiClient client;

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nWake up");

  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  delay(10);
  Serial.println("Initialize DHT sensor");
  dht.begin();
  delay(2000);

  Serial.println("Read DHT sensor");
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  // Convert the floats to strings and round to 2 decimal places
  dtostrf(humidity, 1, 2, str_humidity);
  dtostrf(temperature, 1, 2, str_temperature);
  dtostrf(heatIndex, 1, 2, str_heatIndex);

  Serial.printf("Humidity:    %s %%\nTemperature: %s *C\nHeat index:  %s *C\n", str_humidity, str_temperature, str_heatIndex);

  if (client.connect(server,80)) {
    fnSendToThingspeak(temperature,humidity,heatIndex);
  }
  
  Serial.printf("Sleep for %d seconds\n\n", sleepSeconds);

  // convert to microseconds
  ESP.deepSleep(sleepSeconds * 1000000, WAKE_RF_DEFAULT);
}

void loop() {
}

void fnSendToThingspeak(float t , float h , float hic) {
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(t);
    postStr +="&field2=";
    postStr += String(h);
    postStr +="&field3=";
    postStr += String(hic);    
    postStr += "\r\n\r\n";
 
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println("Sending data to Thingspeak");
}

