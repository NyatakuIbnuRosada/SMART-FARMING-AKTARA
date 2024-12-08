#include <Wire.h>
#include <RTClib.h> // Library RTC

RTC_DS1307 rtc;

// Pin dan parameter
const int soilSensorPin = A0;  // Pin sensor kelembapan tanah
const int relayPin = 8;        // Pin relay pada Arduino Uno
const int threshold = 600;     // Ambang batas kelembapan tanah (sesuaikan dengan nilai Anda)

// Jumlah maksimum jadwal
const int maxSchedules = 10;

// Struktur untuk menyimpan jadwal
struct Schedule {
  int startHour;
  int startMinute;
  int endHour;
  int endMinute;
  bool enabled; // Menandakan apakah jadwal aktif atau tidak
};
Schedule schedules[maxSchedules];
int numSchedules = 0;

// Status relay dan mode kontrol
String relayStatus = "OFF";
bool manualMode = false;

void setup() {
  Serial.begin(9600); // Serial untuk komunikasi dengan NodeMCU

  pinMode(soilSensorPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  if (!rtc.begin()) {
    Serial.println("RTC tidak ditemukan!");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC belum diatur. Mengatur waktu sekarang...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Tambahkan jadwal penyiraman secara manual
  addSchedule(11, 30, 11, 31, true);  // Contoh: nyalakan 8:00-8:15
  addSchedule(18, 0, 18, 15, true); // Contoh: nyalakan 18:00-18:15
}

void loop() {
  DateTime now = rtc.now();

  int currentHour = now.hour();
  int currentMinute = now.minute();

  // Mode manual: periksa perintah dari Serial Monitor
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }

  if (!manualMode) {
    // Mode otomatis: kontrol relay berdasarkan kelembapan tanah dan jadwal RTC
    bool withinSchedule = false;
    for (int i = 0; i < numSchedules; i++) {
      if (schedules[i].enabled &&
          (currentHour > schedules[i].startHour || (currentHour == schedules[i].startHour && currentMinute >= schedules[i].startMinute)) &&
          (currentHour < schedules[i].endHour || (currentHour == schedules[i].endHour && currentMinute < schedules[i].endMinute))) {
        withinSchedule = true;
        break;
      }
    }

    int soilMoisture = analogRead(soilSensorPin); // Baca sensor kelembapan tanah

    if (withinSchedule || soilMoisture > threshold) { 
      relayStatus = "ON"; // Nyalakan relay
      digitalWrite(relayPin, LOW);
    } else {
      relayStatus = "OFF"; // Matikan relay
      digitalWrite(relayPin, HIGH);
    }

    // Tentukan status kelembapan tanah
    String soilStatus = (soilMoisture > threshold) ? "Kering" : "Basah";

    // Format data untuk NodeMCU
    String dataToSend = "Moisture:" + String(soilMoisture) +
                        "|Status:" + soilStatus +
                        "|Time:" + String(currentHour) + ":" + String(currentMinute) +
                        "|Relay:" + relayStatus;

    // Kirim data ke NodeMCU
    Serial.println(dataToSend);
  }

  delay(1000);
}

// Tambahkan jadwal baru
void addSchedule(int startHour, int startMinute, int endHour, int endMinute, bool enabled) {
  if (numSchedules < maxSchedules) {
    schedules[numSchedules] = {startHour, startMinute, endHour, endMinute, enabled};
    numSchedules++;
    Serial.println("Jadwal baru ditambahkan");
  } else {
    Serial.println("Jumlah jadwal maksimal telah tercapai");
  }
}

// Fungsi untuk memproses perintah dari Serial Monitor
void processCommand(String command) {
  if (command.startsWith("Relay:")) {
    String state = command.substring(6);

    if (state == "ON") {
      relayStatus = "ON";
      digitalWrite(relayPin, HIGH);
      manualMode = true;
    } else if (state == "OFF") {
      relayStatus = "OFF";
      digitalWrite(relayPin, LOW);
      manualMode = true;
    }

    Serial.println("Mode Manual Aktif: Relay " + relayStatus);
  } else if (command == "Auto") {
    manualMode = false;
    Serial.println("Mode Otomatis Aktif");
  } else if (command.startsWith("Add:")) {
    // Contoh: Add:6:0:6:15
    int startHour, startMinute, endHour, endMinute;
    sscanf(command.c_str(), "Add:%d:%d:%d:%d", &startHour, &startMinute, &endHour, &endMinute);
    addSchedule(startHour, startMinute, endHour, endMinute, true);
  }
}
