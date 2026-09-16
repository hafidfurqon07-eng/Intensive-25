#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

// ======================================================
// WIFI
// ======================================================

#define WIFI_SSID "Rlnd"
#define WIFI_PASSWORD "punyaiot"

// ======================================================
// FIREBASE
// ======================================================

#define API_KEY "AIzaSyAqESbhHAbQju5iU_1FrU3Gy0nSiOaR5w"
#define DATABASE_URL "https://uvdra-afcd1-default-rtdb.firebaseio.com/"

// ======================================================
// PIN
// ======================================================

#define GUVA_PIN 34

#define SDA_PIN 21
#define SCL_PIN 22

// ======================================================
// FIREBASE OBJECT
// ======================================================

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

// ======================================================
// SHT31
// ======================================================

Adafruit_SHT31 sht31 = Adafruit_SHT31();

// ======================================================
// TIMER
// ======================================================

unsigned long lastSend = 0;
const unsigned long interval = 2000;

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  // GUVA
  pinMode(GUVA_PIN, INPUT);

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // SHT31
  if (!sht31.begin(0x44)) {
    Serial.println("SHT31 tidak ditemukan!");
  } else {
    Serial.println("SHT31 berhasil terhubung!");
  }

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Menghubungkan WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi terhubung!");
  Serial.println(WiFi.localIP());

  // Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signup berhasil");
    signupOK = true;
  } else {
    Serial.println("Firebase signup gagal:");
    Serial.println(config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase siap!");
}

// ======================================================
// LOOP
// ======================================================

void loop() {

  if (Firebase.ready() && signupOK) {

    if (millis() - lastSend >= interval) {

      lastSend = millis();

      // =================================================
      // BACA GUVA
      // =================================================

      int uv = analogRead(GUVA_PIN);

      // =================================================
      // BACA KELEMBAPAN SHT31
      // =================================================

      float kelembapan = sht31.readHumidity();

      // =================================================
      // SERIAL MONITOR
      // =================================================

      Serial.println("==========================");

      Serial.print("UV RAW       : ");
      Serial.println(uv);

      Serial.print("Kelembapan   : ");
      Serial.print(kelembapan, 2);
      Serial.println(" %");

      // =================================================
      // KIRIM UV KE FIREBASE
      // =================================================

      if (Firebase.RTDB.setInt(&fbdo, "/sensor/uv", uv)) {
        Serial.println("UV berhasil dikirim");
      } else {
        Serial.println("Gagal kirim UV");
        Serial.println(fbdo.errorReason());
      }

      // =================================================
      // KIRIM KELEMBAPAN KE FIREBASE
      // =================================================

      if (!isnan(kelembapan)) {

        if (Firebase.RTDB.setFloat(&fbdo, "/sensor/kelembapan", kelembapan)) {
          Serial.println("Kelembapan berhasil dikirim");
        } else {
          Serial.println("Gagal kirim kelembapan");
          Serial.println(fbdo.errorReason());
        }

      } else {
        Serial.println("Pembacaan kelembapan gagal!");
      }

      Serial.println("==========================");
    }
  }
}