#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

// WiFi
#define WIFI_SSID "infinergy"
#define WIFI_PASSWORD "okeokeoke"

// Firebase
#define API_KEY "AIzaSyAj1WuRBgli6srEqC2Itd71H_xAptILN0o"
#define DATABASE_URL "pitrogreensystem-default-rtdb.firebaseio.com"
#define USER_EMAIL "pgsadmin@gmail.com"
#define USER_PASSWORD "Admin123"

// Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200);

// DHT Sensor
#define DHTPIN D5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// BH1750 Sensor
BH1750 lightMeter;

// Soil sensor
#define SOIL_SENSOR A0

// Timing
const long interval = 60000;
unsigned long previousMillis = 0;

// Task Handles
TaskHandle_t SensorTaskHandle;
TaskHandle_t FirebaseTaskHandle;

float temperature, humidity;
int soil, ldr;
String currentDate;

// WiFi connection
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Firebase initialization
void initFirebase() {
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
}

// Sensor reading task
void SensorTask(void *parameter) {
  for (;;) {
    // Time update
    timeClient.update();
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);
    int hoursNow = ptm->tm_hour;
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    int currentYear = ptm->tm_year + 1900;
    currentDate = String(currentYear) + "/" + String(currentMonth) + "/" + String(monthDay) + "/" + String(hoursNow);

    // Sensor readings
    humidity = dht.readHumidity();
    if (isnan(humidity)) humidity = 0;

    temperature = dht.readTemperature();
    if (isnan(temperature)) temperature = 0;

    soil = map(analogRead(SOIL_SENSOR), 1023, 0, 0, 100);

    ldr = lightMeter.readLightLevel();
    if (isnan(ldr)) ldr = 0;

    vTaskDelay(pdMS_TO_TICKS(1000)); /
  }
}

// Firebase task
void FirebaseTask(void *parameter) {
  for (;;) {
    unsigned long currentMillis = millis();
    if (Firebase.ready() && currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Upload sensor data
      if (Firebase.setInt(fbdo, "/data/2/humidity/" + currentDate, humidity)) {
        Serial.println("Humidity uploaded: " + String(humidity));
      } else {
        Serial.println("Error: " + fbdo.errorReason());
      }

      if (Firebase.setInt(fbdo, "/data/2/temperature/" + currentDate, temperature)) {
        Serial.println("Temperature uploaded: " + String(temperature));
      } else {
        Serial.println("Error: " + fbdo.errorReason());
      }

      if (Firebase.setInt(fbdo, "/data/2/soil/" + currentDate, soil)) {
        Serial.println("Soil uploaded: " + String(soil));
      } else {
        Serial.println("Error: " + fbdo.errorReason());
      }

      if (Firebase.setInt(fbdo, "/data/2/light/" + currentDate, ldr)) {
        Serial.println("Light uploaded: " + String(ldr));
      } else {
        Serial.println("Error: " + fbdo.errorReason());
      }

      String paramAll = String(temperature) + String(humidity) + String(soil) + String(ldr);
      if (Firebase.setString(fbdo, "/param/2", paramAll)) {
        Serial.println("Parameters uploaded: " + paramAll);
      } else {
        Serial.println("Error: " + fbdo.errorReason());
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5000)); 
  }
}

void setup() {
  Serial.begin(115200);

  connectWiFi();
  initFirebase();

  timeClient.begin();
  dht.begin();
  Wire.begin();
  lightMeter.begin();

  // FreeRTOS tasks
  xTaskCreate(SensorTask, "Sensor Task", 2048, NULL, 1, &SensorTaskHandle);
  xTaskCreate(FirebaseTask, "Firebase Task", 4096, NULL, 1, &FirebaseTaskHandle);
}

void loop() {
}
