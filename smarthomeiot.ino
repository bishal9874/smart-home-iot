#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>

// WiFi credentials
#define WIFI_SSID "illusion"
#define WIFI_PASSWORD "12345678"

// Firebase credentials
#define API_KEY "AIzaSyAAINNRbth3fbAwXBsxR0NL1AmnWRskKy8"
#define DATABASE_URL "https://smart-home-75464-default-rtdb.firebaseio.com/"
#define USER_EMAIL "femaleaura865@gmail.com"
#define USER_PASSWORD "Female906@"
#define USER_ID "sRWqDuVfO1bvEPwSipuKhSgcNOI2"

// Pin assignments for NodeMCU
const int trigPin = 14; // D5
const int echoPin = 12; // D6
const int ledPin = 4;   // D2

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Firebase paths
String basePath = "/users/" + String(USER_ID);
String autoModePath = basePath + "/autoModes/auto1";
String lightStatePath = basePath + "/lightStates/light1";
String distancePath = basePath + "/distances/measureDistance";
String maxDistancePath = basePath + "/distances/distance1";

// Logic variables
unsigned long motionDetectedTime = 0;
unsigned long lastUpdateTime = 0;
const unsigned long interval = 1000;
const int ledOnTime = 10000;
bool ledOn = false;
bool autoMode = false;
float maxDistance; // default fallback

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  connectToWiFi();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  if (Firebase.ready() && millis() - lastUpdateTime > interval) {
    lastUpdateTime = millis();

    // --- Read autoMode from Firebase
    if (Firebase.RTDB.getBool(&fbdo, autoModePath.c_str())) {
      autoMode = fbdo.boolData();
    }

    // --- Read maxDistance from Firebase
    if (Firebase.RTDB.getFloat(&fbdo, maxDistancePath.c_str())) {
      maxDistance = fbdo.floatData();
    }

    if (autoMode) {
      // --- Auto Mode: Read distance from sensor
      float distance = measureDistance();
      if (distance > 0) {
        Serial.printf("Measured: %.2f cm\n", distance);

        // Update distance to Firebase
        Firebase.RTDB.setFloat(&fbdo, distancePath.c_str(), distance);

        if (distance <= maxDistance) {
          digitalWrite(ledPin, HIGH);
          ledOn = true;
          motionDetectedTime = millis();
        }
      }

      // Turn off LED after timeout
      if (ledOn && millis() - motionDetectedTime >= ledOnTime) {
        digitalWrite(ledPin, LOW);
        ledOn = false;
        Serial.println("LED turned off due to timeout.");
      }

    } else {
      // --- Manual Mode
      if (Firebase.RTDB.getBool(&fbdo, lightStatePath.c_str())) {
        bool lightState = fbdo.boolData();
        digitalWrite(ledPin, lightState);
        ledOn = lightState;
        Serial.printf("Manual LED: %s\n", lightState ? "ON" : "OFF");
      }
    }

    // --- Status Print
    Serial.println("--- STATUS ---");
    Serial.printf("Auto Mode: %s\n", autoMode ? "ON" : "OFF");
    Serial.printf("LED: %s\n", digitalRead(ledPin) ? "ON" : "OFF");
    Serial.printf("maxDistance: %.2f cm\n", maxDistance);
    Serial.println("---------------");
  }
}

float measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  if (duration == 0) return -1;
  float dist = duration * 0.0343 / 2;
  if (dist <= 0 || dist > 400) return -1;
  return dist;
}

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    Serial.print(".");
    delay(300);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected, IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWi-Fi connection failed.");
  }
}
