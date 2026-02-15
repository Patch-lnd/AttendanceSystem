/*
  Attendance System ESP32
  Lecture RFID RC522 + envoi HTTP au serveur Node.js
  Buzzer passif pour feedback sonore
*/

// =====================
// LIBRAIRIES
// =====================
#include <WiFi.h>           // Connexion WiFi
#include <HTTPClient.h>     // Requêtes HTTP
#include <SPI.h>            // Communication SPI pour RC522
#include <MFRC522.h>        // Bibliothèque RC522

// =====================
// WIFI
// =====================
const char* ssid = "Galaxy A23 5G7449";       // Nom de ton réseau WiFi
const char* password = "armj0925";    // Mot de passe WiFi

// =====================
// SERVEUR NODE
// =====================
const char* serverURL = "http://192.168.40.36:3000/api/attendance"; // IP locale Node + route API

// =====================
// BUZZER PASSIF
// =====================
const int buzzerPin = 27; // GPIO où le buzzer est branché

// =====================
// RC522
// =====================
#define RST_PIN  22  // Reset du RC522
#define SS_PIN   21  // Slave Select (SDA)

MFRC522 rfid(SS_PIN, RST_PIN); // Instance RC522

// =====================
// SETUP
// =====================
void setup() {
  Serial.begin(115200);      // Moniteur série pour debug
  SPI.begin();               // Init SPI
  rfid.PCD_Init();           // Init RC522
  pinMode(buzzerPin, OUTPUT); // Buzzer en sortie

  // Connexion WiFi
  Serial.print("Connexion au WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnecté au WiFi !");
}

// =====================
// FONCTION POUR LE BIP
// =====================
void beep(int frequency, int duration) {
  tone(buzzerPin, frequency, duration); // Génère le son
  delay(duration + 50);                 // Petite pause pour que le son s'arrête
}

// =====================
// LOOP PRINCIPAL
// =====================
void loop() {
  // Vérifie si une nouvelle carte est présente
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Conversion UID en string HEX
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString += String(rfid.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();

  Serial.print("Carte détectée : ");
  Serial.println(uidString);

  // Bip court → carte détectée
  beep(2000, 100);

  // =====================
  // ENVOI HTTP POST
  // =====================
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String jsonBody = "{\"rfid_uid\":\"" + uidString + "\"}";

    int httpResponseCode = http.POST(jsonBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Réponse serveur : " + response);

      // Analyse simple JSON pour succès ou échec
      if (response.indexOf("\"status\":\"success\"") >= 0) {
        beep(3000, 150); // Succès → bip aigu
      } else {
        beep(500, 400); // Échec → bip grave
      }

    } else {
      Serial.println("Erreur connexion serveur !");
      for (int i = 0; i < 3; i++) beep(1000, 100); // 3 bips → erreur réseau
    }

    http.end(); // Libération de la connexion HTTP
  } else {
    Serial.println("WiFi non connecté !");
    for (int i = 0; i < 3; i++) beep(1000, 100); // 3 bips → pas de WiFi
  }

  // Arrêt de la carte pour éviter double lecture
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(500); // Petite pause avant prochaine lecture
}
