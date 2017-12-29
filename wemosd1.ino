/* DHT Shield - Simple
 *
 * Example testing sketch for various DHT humidity/temperature sensors
 * Written by ladyada, public domain
 *
 * Depends on Adafruit DHT Arduino library
 * https://github.com/adafruit/DHT-sensor-library
 */

#include "DHT.h"

#define DHTPIN D4     // what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#include <ESP8266WiFi.h>
String apiKey = "xxxxxxxxxxxxxxxxxx"; 
const char* ssid = "TheBananaStand";
const char* password = "xxxxxxxxxx";
const char* server = "api.thingspeak.com";
int numReadings;
WiFiClient client;
// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);
float temparray[6];
float humidityarray[6];
float hicarray[6];
float tempavg;
float humidityavg;
float hicavg;
void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

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
  delay(10);
  Serial.println("DHTxx test!");
  dht.begin();
  numReadings = 0;
  pinMode(BUILTIN_LED, OUTPUT);
}

void loop() {
  // Wait a few seconds between measurements.
  //delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  // float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    digitalWrite(BUILTIN_LED, HIGH);
    return; 
  } else {
    digitalWrite(BUILTIN_LED, LOW);
  }

  // Compute heat index in Fahrenheit (the default)
  // float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  //Serial.print(f);
  //Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  //Serial.print(hif);
  //Serial.println(" *F");
  temparray[numReadings] = t;
  humidityarray[numReadings] = h;
  hicarray[numReadings] = hic;
   if (client.connect(server,80)) {
     if (numReadings == 5) {  // average over the six readings
      // writing the average every 6 readings ~1 minute to match the front gate sensor
      tempavg = (temparray[0]+temparray[1]+temparray[2]+temparray[3]+temparray[4]+temparray[5])/6;
      humidityavg = (humidityarray[0]+humidityarray[1]+humidityarray[2]+humidityarray[3]+humidityarray[4]+humidityarray[5])/6;
      hicavg = (hicarray[0]+hicarray[1]+hicarray[2]+hicarray[3]+hicarray[4]+hicarray[5])/6;
      fnSendToThingspeak(tempavg,humidityavg,hicavg);
      numReadings = 0;
     }
  }
  
  client.stop();
  
  Serial.println("Waiting 60 secs");
  // thingspeak needs at least a 15 sec delay between updates
  // 20 seconds to be safe
  delay(9500);  // 9.5 secs between readings + time required to read temperature and humidity
  numReadings++;
  // Serial.println(numReadings);
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
    numReadings = 0;
}

