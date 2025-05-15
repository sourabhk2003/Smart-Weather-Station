extention type of arduino ide code file contains #include \<ESP8266WiFi.h>
\#include \<DHT.h>
\#include \<Wire.h>
\#include \<hd44780.h>                         // Main LCD library
\#include \<hd44780ioClass/hd44780\_I2Cexp.h>   // I2C I/O class
\#include "Adafruit\_MQTT.h"
\#include "Adafruit\_MQTT\_Client.h"

// WiFi credentials
\#define WIFI\_SSID       "Airtel\_akas\_2104"
\#define WIFI\_PASS       "air30759"

// Adafruit IO credentials
\#define AIO\_USERNAME    "Ishant01275"
\#define AIO\_KEY         "aio\_GSwC05xlVC4iNKlIDQr5T8SyrMQK"

// Pin Definitions
\#define DHTPIN          D5
\#define DHTTYPE         DHT11
\#define RAIN\_PIN        D6
\#define LDR\_PIN         A0
\#define LED\_PIN         D7

// Objects
DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
Adafruit\_MQTT\_Client mqtt(\&client, "io.adafruit.com", 1883, AIO\_USERNAME, AIO\_KEY);

// LCD object
hd44780\_I2Cexp lcd;

// Feeds
Adafruit\_MQTT\_Publish tempFeed  = Adafruit\_MQTT\_Publish(\&mqtt, AIO\_USERNAME "/feeds/temperature");
Adafruit\_MQTT\_Publish humidFeed = Adafruit\_MQTT\_Publish(\&mqtt, AIO\_USERNAME "/feeds/humidity");
Adafruit\_MQTT\_Publish lightFeed = Adafruit\_MQTT\_Publish(\&mqtt, AIO\_USERNAME "/feeds/light");
Adafruit\_MQTT\_Publish rainFeed  = Adafruit\_MQTT\_Publish(\&mqtt, AIO\_USERNAME "/feeds/rain");

// Limits for the sensors (Safe Limits)
\#define TEMP\_MIN        10    // Minimum temperature in Celsius
\#define TEMP\_MAX        35    // Maximum temperature in Celsius
\#define HUMID\_MIN       20    // Minimum humidity percentage
\#define HUMID\_MAX       80    // Maximum humidity percentage
\#define LIGHT\_MIN       10  // Minimum light level (LDR value)
\#define LIGHT\_MAX       35   // Maximum light level (LDR value)

// Variables to track if alerts have been shown
bool tempLowAlertShown = false;
bool tempHighAlertShown = false;

void connectToAdafruit() {
int8\_t ret;
while ((ret = mqtt.connect()) != 0) {
Serial.print("Adafruit IO connection failed, retrying...");
mqtt.disconnect();
delay(2000);
}
Serial.println("Connected to Adafruit IO");
}

void setup() {
Serial.begin(9600);
delay(10);

pinMode(RAIN\_PIN, INPUT);
pinMode(LED\_PIN, OUTPUT);
dht.begin();

// WiFi connection
Serial.print("Connecting to Wi-Fi");
WiFi.begin(WIFI\_SSID, WIFI\_PASS);
while (WiFi.status() != WL\_CONNECTED) {
delay(500); Serial.print(".");
}
Serial.println("\nConnected to Wi-Fi");

// MQTT
connectToAdafruit();

// LCD init
if (lcd.begin(16, 2) == 0) {
lcd.backlight();
lcd.setCursor(0, 0);
lcd.print("Weather Station");
lcd.setCursor(0, 1);
lcd.print("Initializing...");
delay(2000);
} else {
Serial.println("LCD init failed!");
}
}

void loop() {
if (!mqtt.connected()) {
connectToAdafruit();
}
mqtt.processPackets(10000);

float temp = dht.readTemperature();
float humid = dht.readHumidity();
int lightVal = analogRead(LDR\_PIN);
int rainVal = digitalRead(RAIN\_PIN); // 0 = Rain Detected

// Weather Station Report:
Serial.println("\nWeather Station Report:");

// Temperature Reading
Serial.print("Temperature: ");
Serial.print(temp);
Serial.print(" °C");
if (temp < TEMP\_MIN && !tempLowAlertShown) {
Serial.println(" - Alert! It's getting too cold. Stay warm!");
tempLowAlertShown = true;  // Ensure alert is only shown once
} else if (temp > TEMP\_MAX && !tempHighAlertShown) {
Serial.println(" - Alert! It's getting too hot. Stay cool!");
tempHighAlertShown = true;  // Ensure alert is only shown once
} else {
if (tempLowAlertShown || tempHighAlertShown) {
tempLowAlertShown = false;
tempHighAlertShown = false;
}
Serial.println(" - The weather is just perfect. Enjoy your day!");
}

// Humidity Reading
Serial.print("Humidity: ");
Serial.print(humid);
Serial.print(" %");
if (humid < HUMID\_MIN) {
Serial.println(" - Alert! It's too dry, make sure to stay hydrated!");
} else if (humid > HUMID\_MAX) {
Serial.println(" - Alert! High humidity, take a cool break!");
} else {
Serial.println(" - Moderate humidity. Perfect for outdoor activities.");
}

// Light Level Reading
Serial.print("Light Level: ");
Serial.print(lightVal);
if (lightVal < LIGHT\_MIN) {
Serial.println(" - Alert! It's getting too dark. Turn on some lights!");
} else if (lightVal > LIGHT\_MAX) {
Serial.println(" - Alert! It's too bright, put on some shades!");
} else {
Serial.println(" - Light conditions are just right.");
}

// Rain Status Reading
Serial.print("Rain Status: ");
if (rainVal == 0) {
Serial.println("Alert! Rain detected! Stay dry.");
} else {
Serial.println("No rain detected.");
}

// LCD Display
lcd.clear();
lcd.setCursor(0, 0);
lcd.print("T:"); lcd.print(temp); lcd.print(" H:"); lcd.print(humid);
lcd.setCursor(0, 1);
lcd.print("L:"); lcd.print(lightVal); lcd.print(" R:"); lcd.print(rainVal == 0 ? "Y" : "N");

// Publish to Adafruit IO
if (!isnan(temp))   tempFeed.publish(temp);
if (!isnan(humid))  humidFeed.publish(humid);
lightFeed.publish(lightVal);
rainFeed.publish(rainVal == 0 ? 1 : 0);

// LED Indicator
if (rainVal == 0) digitalWrite(LED\_PIN, HIGH);
else digitalWrite(LED\_PIN, LOW);

delay(1000); // Reduced delay for faster sensor updates
}
