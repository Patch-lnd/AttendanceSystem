// RFID RC522 Libraries
#include <SPI.h> // SPI communication série
#include <MFRC522.h> // Module RFID RC522

// LCD Libraries 
#include <Wire.h> // Communication I2C pour la LCD
#include <LiquidCrystal_I2C.h> // Affichage sur un écran LCD

#include <WiFi.h> // Connexion WiFi pour ESP32
#include <HTTPClient.h> // Communication HTTP pour ESP32


/* --------------------------------------------------------------------
   TABLEAU DE CABLAGE POUR ESP32 - PROJET RFID + LCD + WiFi + LEDS + BUZZER
   --------------------------------------------------------------------

   ┌──────────────────────┬────────────────────┬──────────────────────────────┬──────────────────────────────┐
   │ Composant            │ Broche sur ESP32   │ Fonction dans le Code        │ Remarques                    │
   ├──────────────────────┼────────────────────┼──────────────────────────────┼──────────────────────────────┤
   │ RC522 RFID           │ GPIO 32            │ SS_PIN                       │ Sélection SPI                 │
   │                      │ GPIO 33            │ RST_PIN                      │ Reset du module               │
   │                      │ GPIO 18            │ SCK                          │ Clock SPI par défaut ESP32    │
   │                      │ GPIO 23            │ MOSI                         │ Master Out Slave In (SPI)     │
   │                      │ GPIO 19            │ MISO                         │ Master In Slave Out (SPI)     │
   │                      │ 3.3V               │ VCC                          │ Alimentation (⚠ pas 5V)       │
   │                      │ GND                │ GND                          │ Masse                         │
   ├──────────────────────┼────────────────────┼──────────────────────────────┼──────────────────────────────┤
   │ LCD I2C (16x2)       │ GPIO 21            │ SDA                          │ Communication I2C             │
   │                      │ GPIO 22            │ SCL                          │ Communication I2C             │
   │                      │ 5V (ou 3.3V)       │ VCC                          │ Alim (selon le modèle)        │
   │                      │ GND                │ GND                          │ Masse                         │
   ├──────────────────────┼────────────────────┼──────────────────────────────┼──────────────────────────────┤
   │ LED Jaune            │ GPIO 25            │ LED_JAUNE                    │ Indicateur d’activité         │
   │ LED Bleue            │ GPIO 26            │ LED_BLEUE                    │ Indicateur de lecture         │
   │ Buzzer               │ GPIO 27            │ BUZZER                       │ Son de notification           │
   └──────────────────────┴────────────────────┴──────────────────────────────┴──────────────────────────────┘

   NOTES :
   - Les broches SPI sont par défaut sur l'ESP32 : SCK=18, MOSI=23, MISO=19
   - Le LCD utilise le protocole I2C : SDA=21, SCL=22 (initialisé avec Wire.begin(21, 22))
   - Le RC522 doit être alimenté en 3.3V uniquement
   - Ajouter une résistance de 220Ω pour chaque LED si nécessaire
   - Toutes les masses (GND) doivent être connectées ensemble (module + ESP32 + alim)
--------------------------------------------------------------------- */

#define WIFI_SSID "TECNO POP 9"         // SSID WiFi
#define WIFI_PASSWORD "maison88"      // Mot de passe WiFi
#define SERVER_URL "http://192.168.207.70:3000/transactions" // Endpoint serveur

// Définition des broches du module RFID
#define RST_PIN 33 // Pin de reset du RC522
#define SS_PIN 32  // Pin de sélection du RC522

MFRC522 rfid(SS_PIN, RST_PIN); // Créer une instance de MFRC522
LiquidCrystal_I2C lcd(0x27, 16, 2); // Écran LCD 16x2 sur I2C

// Définition des broches des composants
#define BUZZER 27     // Buzzer
#define LED_BLEUE 26  // LED bleue
#define LED_JAUNE 25  // LED jaune

void setup() {
  Serial.begin(115200); // Initialise le port série pour debug
  SPI.begin();          // Initialise le bus SPI
  rfid.PCD_Init();      // Initialise le lecteur RFID

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Connexion WiFi
  Serial.print("Connexion au WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connecté au WiFi !"); // Confirmation WiFi

  // Initialisation de l'I2C pour l'écran LCD (SDA=21, SCL=22)
  Wire.begin(21, 22);
  lcd.init();       // Initialise l'écran LCD
  lcd.backlight();  // Allume le rétro-éclairage

  // Configure les broches des composants en sortie
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_BLEUE, OUTPUT);
  pinMode(LED_JAUNE, OUTPUT);

  // Éteindre buzzer & LEDs
  digitalWrite(BUZZER, LOW);
  digitalWrite(LED_BLEUE, LOW);
  digitalWrite(LED_JAUNE, LOW);
  delay(1000); // Pause d'une seconde

  // Affiche le message initial
  lcd.setCursor(0, 0);
  lcd.println("Scanner carte...");
  Serial.println("Scanner une carte...");
}

// Fonction pour lire l'UID de la carte RFID
String readUID() {
  String uid = "";
  digitalWrite(LED_JAUNE, HIGH); // Allume LED jaune
  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

// Fonction pour authentifier un secteur avant lecture
bool authSector(byte sector, byte block) {
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  MFRC522::StatusCode status = rfid.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, sector * 4 + block, &key, &(rfid.uid)
  );
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Erreur d'authentification sur le secteur");
    return false;
  }
  return true;
}

// Fonction pour lire un bloc de la carte RFID
/* String readBlock(byte sector, byte block) {
  byte buffer[18];
  byte size = sizeof(buffer);
  String data = "";
  if (authSector(sector, block)) {
    MFRC522::StatusCode status = rfid.MIFARE_Read(sector * 4 + block, buffer, &size);
    if (status == MFRC522::STATUS_OK) {
      for (int i = 0; i < 16; i++) {
        if (buffer[i] != 0) data += (char)buffer[i];
      }
    } else {
      Serial.println("Erreur de lecture du bloc");
      delay(2000);
      // COUPE LA COMMUNICATION RPGID POUR LA CARTE PRÉSENTE
      rfid.PICC_HaltA();         // Stop communication
      rfid.PCD_StopCrypto1();    // Désactive la cryptographie
    }
  }
  return data;
} */

// Fonction pour lire la carte jusqu'à 3 blocs vides
String readCard() {
  String fullData = "";
  byte sector = 4;
  byte block = 0;
  byte emptyCount = 0;
  while (sector < 16) {
    String data = readBlock(sector, block);
    if (data.length() == 0) emptyCount++;
    else emptyCount = 0;
    fullData += data;
    if (emptyCount >= 3) break;
    block++;
    if (block > 3) {
      block = 0;
      sector++;
    }
  }
  return fullData;
}

// Fonction pour envoyer UID, montant et PIN au serveur
void sendTransaction(String uid) {
  // Affiche dans le moniteur série la demande du montant à saisir
  Serial.println("Entrez le montant:");

  // Efface l'écran LCD pour préparer un nouvel affichage
  lcd.clear();

  // Place le curseur au début de la première ligne de l'écran LCD
  lcd.setCursor(0, 0);

  // Affiche "Montant FCFA:" sur l'écran LCD, invite à saisir le montant
  lcd.print("Montant FCFA:");

  // Attend que des données soient disponibles sur le port série (utilisateur entre montant)
  while (Serial.available() == 0);

  // Lit la chaîne de caractères saisie jusqu'au saut de ligne ('\n')
  String amount = Serial.readStringUntil('\n');

  // Supprime les espaces ou caractères invisibles en début et fin de chaîne
  amount.trim();

  // Affiche dans le moniteur série la demande de saisie du PIN
  Serial.println("Entrez le PIN (6):");

  // Efface l'écran LCD pour préparer le message du PIN
  lcd.clear();

  // Positionne le curseur au début de la première ligne LCD
  lcd.setCursor(0, 0);

  // Affiche "Code PIN(6):" pour inviter à saisir le code PIN
  lcd.print("Code PIN(6):");

  // Attend que des données soient disponibles sur le port série (saisie du PIN)
  while (Serial.available() == 0);

  // Lit la chaîne de caractères saisie du PIN jusqu'au saut de ligne
  String pin = Serial.readStringUntil('\n');

  // Nettoie la chaîne PIN des espaces ou caractères invisibles
  pin.trim();
  // Crée un objet HTTPClient qu

  // Crée un objet HTTPClient qui gère la communication HTTP
  HTTPClient http;

  // Initialise la connexion HTTP vers l'adresse du serveur spécifiée dans SERVER_URL
  http.begin(SERVER_URL);

  // Ajoute l'en-tête HTTP "Content-Type" pour préciser que les données envoyées sont en URL-encoded
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // --- MODIF ---
  // Prépare la chaîne des données à envoyer en POST :
  // inclut card_uid (UID de la carte), pin, amount et from=esp32 pour indiquer la source
  String postData = "card_uid=" + uid + "&pin=" + pin + "&amount=" + amount + "&from=esp32";

  // --- MODIF ---
  // Ajoute un en-tête HTTP "Accept" pour dire qu'on attend une réponse en JSON du serveur
  http.addHeader("Accept", "application/json");

  // Efface l'écran LCD pour afficher le message d'envoi
  lcd.clear();

  // Affiche "Envoi..." sur le LCD pendant que la requête est envoyée
  lcd.print("Envoi...");

  // Enregistre le temps actuel en millisecondes (pour mesurer la durée de la requête)
  unsigned long start = millis();

  // Envoie la requête POST avec les données postData et récupère le code HTTP reçu (ex : 200, 404, etc.)
  int httpCode = http.POST(postData);

  // Calcule la durée écoulée depuis l'envoi de la requête
  unsigned long duration = millis() - start;

  // Efface l'écran LCD pour afficher la réponse ou un message d'erreur
  lcd.clear();

  // Si la requête a mis plus de 10 secondes à répondre ou n'a pas réussi (code <= 0)
  if (duration > 10000 || httpCode <= 0) {
    // Affiche "Timeout !" sur l'écran LCD
    lcd.print("Timeout !");

    // Affiche dans le moniteur série que la requête a expiré (plus de 10 secondes)
    Serial.println("Timeout (>10s)");
  } else {
    // Sinon récupère la chaîne du corps de la réponse HTTP (texte envoyé par le serveur)
    String payload = http.getString();

    // Affiche dans le moniteur série le code HTTP et le contenu reçu
    Serial.println("HTTP " + String(httpCode) + ": " + payload);

    // Si le code HTTP est 200, c'est que la requête a réussi côté serveur
    if (httpCode == 200) {
      // Cherche dans le texte reçu certains mots-clés pour déterminer le message à afficher

      // Si le message contient "Success"
      if (payload.indexOf("Success") >= 0) {
        // Affiche "Transaction OK" sur le LCD
        lcd.print("Transaction OK");

        // Active le buzzer à 1200Hz pendant 300ms pour signaler succès
        tone(BUZZER, 1200, 300);
      }
      // Si le message contient "Invalid PIN"
      else if (payload.indexOf("Invalid PIN") >= 0) {
        // Affiche "PIN incorrect" sur le LCD
        lcd.print("PIN incorrect");
      }
      // Si le message contient "Insufficient" (solde insuffisant)
      else if (payload.indexOf("Insufficient") >= 0) {
        // Affiche "Solde bas" sur le LCD
        lcd.print("Solde bas");
      }
      else {
        // Sinon, affiche la réponse en deux lignes sur l'écran LCD
        lcd.print(payload.substring(0, 16));  // Première ligne : 16 premiers caractères
        lcd.setCursor(0, 1);                   // Deuxième ligne
        lcd.print(payload.substring(16));     // Suite du message
      }
    }
    // Si le code HTTP est 404 (ressource non trouvée)
    else if (httpCode == 404) {
      // Affiche "Carte inconnue" sur le LCD
      lcd.print("Carte inconnue");
    }
    // Pour tous les autres codes d'erreur HTTP
    else {
      // Affiche "Erreur " suivi du code sur le LCD
      lcd.print("Erreur " + String(httpCode));
    }
  }

  // Termine la connexion HTTP et libère les ressources associées
  http.end();

  // Pause de 3 secondes pour laisser le temps à l'utilisateur de lire le message
  delay(3000);
}


// boucle principale
void loop() {
  // Vérifie la présence d'une carte
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    delay(200);
    return;
  }

  // Signale la détection de carte
  Serial.println("Carte détectée !");
  digitalWrite(LED_JAUNE, HIGH);
  tone(BUZZER, 60, 100);
  delay(200);
  digitalWrite(LED_JAUNE, LOW);
  tone(BUZZER, 60, 100);
  delay(200);
  digitalWrite(LED_JAUNE, HIGH);
 
  
  lcd.clear();

  // Lit l'UID
  String uid = readUID();
  Serial.print("UID de la carte: ");
  Serial.println(uid);

  // Lit le contenu de la carte
  String cardData = readCard();
  digitalWrite(LED_JAUNE, LOW);
  digitalWrite(LED_BLEUE, HIGH); 
  delay(1000);
  Serial.println("Contenu: " + cardData);
  digitalWrite(LED_BLEUE, LOW);

  // Affiche les données sur LCD
  lcd.setCursor(0, 0);
  lcd.print(cardData.substring(0,16));
  lcd.setCursor(0, 1);
  lcd.print(cardData.substring(16));
  delay(3000);

  // ENVOI DE LA TRANSACTION
  sendTransaction(uid);

  // COUPE LA COMMUNICATION RPGID POUR LA CARTE PRÉSENTE
  rfid.PICC_HaltA();         // Stop communication
  rfid.PCD_StopCrypto1();    // Désactive la cryptographie

  // PRÉPARE POUR LA PROCHAINE CARTE
  lcd.clear(); lcd.print("Scanner carte...");
  Serial.println("Scanner une carte...");
}
