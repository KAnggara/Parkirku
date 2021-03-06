#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define RL 2

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Kos_oren"
#define WIFI_PASSWORD "masihyanglama"

/* 2. Define the API Key */
#define API_KEY "AIzaSyCIU3o4tt44ZAqqh-TbtjScdMZ0n21GEqs"

/* 3. Define the RTDB URL */
#define DATABASE_URL "parkirku1-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int intValue;
bool signupOK = false;

void setup() {
  Serial.begin(115200);
  pinMode(RL, OUTPUT);
  digitalWrite(RL, LOW);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
int last = 0;

void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    if (Firebase.RTDB.getString(&fbdo, "/ParkirKu/io")) {
      intValue = fbdo.stringData().toInt();
      Serial.println(intValue);
      if (intValue == 1 && last == 0) {
        digitalWrite(RL, HIGH);
        if (Firebase.RTDB.getString(&fbdo, "/ParkirKu/uid")) {
          String uid = fbdo.stringData();
          Serial.println("UID => " + uid);
        }
        last = 1;
      } else if (intValue == 0 && last == 1) {
        last = 0;
        digitalWrite(RL, LOW);
      }

    } else {
      Serial.println(fbdo.errorReason());
    }
  }
}
