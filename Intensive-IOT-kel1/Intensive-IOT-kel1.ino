#include <WiFi.h>
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
// FIREBASE OBJECT
// ======================================================

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

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

  // ====================================================
  // WIFI
  // ====================================================

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Menghubungkan WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ====================================================
  // FIREBASE
  // ====================================================

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

  // ====================================================
  // RANDOM SEED
  // ====================================================

  randomSeed(analogRead(34));

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
      // GENERATE DATA RANDOM
      // =================================================

      float uv = random(0, 120) / 10.0;
      float kelembapan = random(400, 801) / 10.0;
      float tewl = random(50, 201) / 100.0;

      // =================================================
      // SERIAL MONITOR
      // =================================================

      Serial.println("======================");

      Serial.print("UV          : ");
      Serial.println(uv, 1);

      Serial.print("Kelembapan  : ");
      Serial.print(kelembapan, 1);
      Serial.println(" %");

      Serial.print("TEWL        : ");
      Serial.print(tewl, 2);
      Serial.println(" g/m2/h");

      // =================================================
      // KIRIM UV
      // =================================================

      if (Firebase.RTDB.setFloat(&fbdo, "/sensor/uv", uv)) {

        Serial.println("UV terkirim");

      } else {

        Serial.println("Gagal kirim UV");
        Serial.println(fbdo.errorReason());

      }

      // =================================================
      // KIRIM KELEMBAPAN
      // =================================================

      if (Firebase.RTDB.setFloat(&fbdo, "/sensor/kelembapan", kelembapan)) {

        Serial.println("Kelembapan terkirim");

      } else {

        Serial.println("Gagal kirim kelembapan");
        Serial.println(fbdo.errorReason());

      }

      // =================================================
      // KIRIM TEWL
      // =================================================

      if (Firebase.RTDB.setFloat(&fbdo, "/sensor/tewl", tewl)) {

        Serial.println("TEWL terkirim");

      } else {

        Serial.println("Gagal kirim TEWL");
        Serial.println(fbdo.errorReason());

      }

      Serial.println("======================");
    }
  }
}