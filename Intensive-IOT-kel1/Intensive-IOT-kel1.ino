#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

// ======================================================
// WIFI
// ======================================================

#define WIFI_SSID "Rlnd"
#define WIFI_PASSWORD "ininyadigantidulumas"

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
const unsigned long interval = 2000;

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

  // Random seed
  randomSeed(micros());

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
      // DATA SIMULASI
      // =================================================

      // Hydration Level: 0 - 100 %
      float hydrationLevel = random(450, 901) / 10.0;

      // TEWL: g/m²/h
      float tewl = random(50, 201) / 100.0;

      // UV Index: 0 - 11
      float uvIndex = random(0, 121) / 10.0;

      // =================================================
      // SERIAL MONITOR
      // =================================================

      Serial.println();
      Serial.println("==============================");

      Serial.print("Hydration Level : ");
      Serial.print(hydrationLevel, 1);
      Serial.println(" %");

      Serial.print("TEWL            : ");
      Serial.print(tewl, 2);
      Serial.println(" g/m2/h");

      Serial.print("UV Index        : ");
      Serial.println(uvIndex, 1);

      // =================================================
      // KIRIM HYDRATION LEVEL
      // =================================================

      if (Firebase.RTDB.setFloat(
            &fbdo,
            "/sensor/hydration_level",
            hydrationLevel)) {

        Serial.println("Hydration berhasil dikirim");

      } else {

        Serial.println("Gagal kirim Hydration");
        Serial.println(fbdo.errorReason());

      }

      // =================================================
      // KIRIM TEWL
      // =================================================

      if (Firebase.RTDB.setFloat(
            &fbdo,
            "/sensor/tewl",
            tewl)) {

        Serial.println("TEWL berhasil dikirim");

      } else {

        Serial.println("Gagal kirim TEWL");
        Serial.println(fbdo.errorReason());

      }

      // =================================================
      // KIRIM UV INDEX
      // =================================================

      if (Firebase.RTDB.setFloat(
            &fbdo,
            "/sensor/uv_index",
            uvIndex)) {

        Serial.println("UV Index berhasil dikirim");

      } else {

        Serial.println("Gagal kirim UV Index");
        Serial.println(fbdo.errorReason());

      }

      Serial.println("==============================");
    }
  }
}