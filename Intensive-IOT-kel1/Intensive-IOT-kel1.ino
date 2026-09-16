#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

// ======================================================
// WIFI
// ======================================================

#define WIFI_SSID "hafeddd"
#define WIFI_PASSWORD "12345678"

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
const unsigned long interval = 2000; // kirim setiap 2 detik

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  // --------------------------
  // GUVA
  // --------------------------

  pinMode(GUVA_PIN, INPUT);

  // --------------------------
  // I2C
  // --------------------------

  Wire.begin(SDA_PIN, SCL_PIN);

  // --------------------------
  // SHT31
  // --------------------------

  if (!sht31.begin(0x44)) {
    Serial.println("SHT31 tidak ditemukan!");
  } else {
    Serial.println("SHT31 berhasil terhubung.");
  }

  // --------------------------
  // WIFI
  // --------------------------

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Menghubungkan WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi terhubung!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // --------------------------
  // FIREBASE
  // --------------------------

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Anonymous sign up
  if (Firebase.signUp(&config, &auth, "", "")) {

    Serial.println("Firebase signup berhasil");
    signupOK = true;

  } else {

    Serial.println("Firebase signup gagal:");
    Serial.println(config.signer.signupError.message.c_str());

  }

  // Token callback
  config.token_status_callback = tokenStatusCallback;

  // Firebase begin
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

      // ==================================================
      // BACA GUVA
      // ==================================================

      int guvaRaw = analogRead(GUVA_PIN);

      // Konversi ADC ke tegangan
      float guvaVoltage = (guvaRaw / 4095.0) * 3.3;

      // ==================================================
      // BACA SHT31
      // ==================================================

      float suhu = sht31.readTemperature();
      float kelembapan = sht31.readHumidity();

      // ==================================================
      // SERIAL MONITOR
      // ==================================================

      Serial.println("==========================");

      Serial.print("GUVA RAW       : ");
      Serial.println(guvaRaw);

      Serial.print("GUVA Voltage   : ");
      Serial.print(guvaVoltage, 3);
      Serial.println(" V");

      Serial.print("Suhu           : ");
      Serial.print(suhu, 2);
      Serial.println(" °C");

      Serial.print("Kelembapan     : ");
      Serial.print(kelembapan, 2);
      Serial.println(" %");

      // ==================================================
      // KIRIM GUVA RAW
      // ==================================================

      if (Firebase.RTDB.setInt(&fbdo, "/sensor/guva", guvaRaw)) {

        Serial.println("GUVA terkirim");

      } else {

        Serial.println("Gagal kirim GUVA");
        Serial.println(fbdo.errorReason());
      }

      // ==================================================
      // KIRIM TEGANGAN GUVA
      // ==================================================

      if (Firebase.RTDB.setFloat(&fbdo, "/sensor/guva_voltage", guvaVoltage)) {

        Serial.println("GUVA voltage terkirim");

      } else {

        Serial.println("Gagal kirim GUVA voltage");
        Serial.println(fbdo.errorReason());
      }

      // ==================================================
      // KIRIM SUHU
      // ==================================================

      if (!isnan(suhu)) {

        if (Firebase.RTDB.setFloat(&fbdo, "/sensor/suhu", suhu)) {

          Serial.println("Suhu terkirim");

        } else {

          Serial.println("Gagal kirim suhu");
          Serial.println(fbdo.errorReason());
        }
      }

      // ==================================================
      // KIRIM KELEMBAPAN
      // ==================================================

      if (!isnan(kelembapan)) {

        if (Firebase.RTDB.setFloat(&fbdo, "/sensor/kelembapan", kelembapan)) {

          Serial.println("Kelembapan terkirim");

        } else {

          Serial.println("Gagal kirim kelembapan");
          Serial.println(fbdo.errorReason());
        }
      }

      Serial.println("==========================");
    }
  }
}