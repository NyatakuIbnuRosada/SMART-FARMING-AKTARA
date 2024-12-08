#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6sut8ZSf3"
#define BLYNK_TEMPLATE_NAME "SmartFarming"
#define BLYNK_AUTH_TOKEN "ich4JxbagqaJAZJndUXA3U9jfWs7fAMu"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Blynk Authentication Token (gantikan dengan token Anda)
char auth[] = "ich4JxbagqaJAZJndUXA3U9jfWs7fAMu";


// WiFi credentials
char ssid[] = ""; // Masukan Username WIFI
char pass[] = ""; // Masukan password WIFI

// Pin NodeMCU
#define RELAY_PIN 8 // Pin relay
#define SENSOR_PIN A0 // Pin sensor kelembapan tanah

int manualRelayControl = 0; // Variabel untuk menyimpan status kontrol manual

BlynkTimer timer; // Timer untuk pembacaan sensor

void setup() {
  // Inisialisasi Serial dan WiFi
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  // Konfigurasi pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Matikan relay di awal
  pinMode(SENSOR_PIN, INPUT);

  // Set timer untuk pembacaan sensor setiap 1 detik
  timer.setInterval(1000L, readSoilMoisture);

  Serial.println("NodeMCU terhubung ke Blynk.");
}

void loop() {
  Blynk.run(); // Menjaga koneksi Blynk tetap aktif
  timer.run(); // Jalankan timer
}

// Fungsi untuk membaca sensor kelembapan tanah
void readSoilMoisture() {
  int soilMoistureValue = analogRead(SENSOR_PIN); // Baca nilai sensor
  Serial.print("Kelembapan Tanah: ");
  Serial.println(soilMoistureValue);

  // Kirim nilai kelembapan ke Blynk (Virtual Pin V1)
  Blynk.virtualWrite(V1, soilMoistureValue);

  // Jika kontrol manual dimatikan (manualRelayControl == 0), kontrol otomatis aktif
  if (manualRelayControl == 0) {
    if (soilMoistureValue > 600) { // Ambang batas tanah kering
      digitalWrite(RELAY_PIN, HIGH); // Nyalakan relay
      Blynk.virtualWrite(V1, 1);    // Update status relay di Blynk
    } else {
      digitalWrite(RELAY_PIN, LOW); // Matikan relay
      Blynk.virtualWrite(V1, 0);    // Update status relay di Blynk
    }
  }
}

// Fungsi untuk kontrol manual relay dari Blynk (Virtual Pin V3)
BLYNK_WRITE(V0) {
  manualRelayControl = param.asInt(); // Baca nilai kontrol manual dari Blynk (0 = otomatis, 1 = manual)

  if (manualRelayControl == 1) {
    digitalWrite(RELAY_PIN, HIGH); // Nyalakan relay
    Blynk.virtualWrite(V0, 1);    // Update status relay di Blynk
  } else {
    digitalWrite(RELAY_PIN, LOW); // Matikan relay
    Blynk.virtualWrite(V0, 0);    // Update status relay di Blynk
  }
}
