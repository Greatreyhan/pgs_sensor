#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DHT.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// WiFi
const char *ssid = "infinergy";     // Enter your WiFi name
const char *password = "okeokeoke"; // Enter WiFi password

WiFiClient espClient;

/* 1. Define the WiFi credentials */
#define WIFI_SSID "infinergy"
#define WIFI_PASSWORD "okeokeoke"

/* 2. Define the API Key */
#define API_KEY "AIzaSyAj1WuRBgli6srEqC2Itd71H_xAptILN0o"

/* 3. Define the RTDB URL */
#define DATABASE_URL "pitrogreensystem-default-rtdb.firebaseio.com"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "pgsadmin@gmail.com"
#define USER_PASSWORD "Admin123"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define DHTPIN D6
#define SOIL_SENSOR A0
#define LDR_SENSOR D5
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

unsigned long sendDataPrevMillis = 0;
unsigned long previousMillis = 0;
const long interval = 60000;

int count = 0;

void setup()
{

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);

  timeClient.begin();
  timeClient.setTimeOffset(25200);

  dht.begin();
  pinMode(LDR_SENSOR, INPUT);
}

void loop()
{
  // For Timing-----------------------------
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);
  int hoursNow = ptm->tm_hour;
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  int currentYear = ptm->tm_year + 1900;
  String currentDate = String(currentYear) + "/" + String(currentMonth) + "/" + String(monthDay) + "/" + String(hoursNow);
  unsigned long currentMillis = millis();
  float humidity = dht.readHumidity();
  if (isnan(humidity))
    humidity = 0;
  float temperature = dht.readTemperature();
  if (isnan(temperature))
    temperature = 0;
  int soil = map(analogRead(SOIL_SENSOR), 1023, 0, 0, 100);
  int ldr = map(analogRead(LDR_SENSOR), 1023, 0, 0, 100);
  String paramAll = (((int)temperature < 100) ? "0" + String((int)temperature) : ((int)temperature < 10) ? "00" + String((int)temperature)
                                                                                                         : String((int)temperature)) +
                    (((int)humidity < 100) ? "0" + String((int)humidity) : ((int)humidity < 10) ? "00" + String((int)humidity)
                                                                                                : String((int)humidity)) +
                    (((int)soil < 10) ? "00" + String((int)soil) : String((int)soil)) + (((int)ldr < 100) ? "0" + String((int)ldr) : ((int)ldr < 10) ? "00" + String((int)ldr)
                                                                                                                                                     : String((int)ldr));

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    if (Firebase.ready())
    {
      if (Firebase.setInt(fbdo, "/data/1/humidity/" + currentDate, humidity))
      {

        Serial.println(fbdo.dataPath());

        Serial.println(fbdo.pushName());

        Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());
      }
      else
      {
        Serial.println(fbdo.errorReason());
      }
      if (Firebase.setInt(fbdo, "/data/1/temperature/" + currentDate, temperature))
      {

        Serial.println(fbdo.dataPath());

        Serial.println(fbdo.pushName());

        Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());
      }
      else
      {
        Serial.println(fbdo.errorReason());
      }
      if (Firebase.setInt(fbdo, "/data/1/soil/" + currentDate, soil))
      {

        Serial.println(fbdo.dataPath());

        Serial.println(fbdo.pushName());

        Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());
      }
      else
      {
        Serial.println(fbdo.errorReason());
      }

      if (Firebase.setInt(fbdo, "/data/1/light/" + currentDate, ldr))
      {

        Serial.println(fbdo.dataPath());

        Serial.println(fbdo.pushName());

        Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());
      }
      else
      {
        Serial.println(fbdo.errorReason());
      }
    }
  }
  if (Firebase.ready())
  {
    if (Firebase.setString(fbdo, "/param/1", paramAll))
    {
      Serial.println("T : " + String(temperature) + " | H: " + String(humidity) + " | S: " + String(soil) + " | L: " + String(ldr));
      Serial.println(fbdo.dataPath());

      Serial.println(fbdo.pushName());

      Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());
    }
    else
    {
      Serial.println(fbdo.errorReason());
    }
  }
}
