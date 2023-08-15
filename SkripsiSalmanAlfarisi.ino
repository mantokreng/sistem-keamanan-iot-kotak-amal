// Kode program rancang bangun sistem keamanan kotak amal berbasis wemos dan internet of things

// library yang digunakan
#include <ESP8266WiFi.h> // koneksi ke internet
#include <ThingESP.h> // koneksi ke ThingESP
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h> // fingerprint
#include <LiquidCrystal_I2C.h> // lcd + 12c
#include <Servo.h> // motor servo
#include <Wire.h> // kabel

// konfigurasi pin yang digunakan
const int TRIGPIN = D5;          
const int ECHOPIN = D6;
const int pinBuzzer = D7; 
const int pinServo = D8;
SoftwareSerial mySerial(D1,D3); // TX/RX pada fingerprint

// deklarasi variabel
long timer;
int jarak;
uint8_t id;
Servo motorservo;


// inisialisasi kondisi tetap dari sistem
ThingESP8266 thing("kingsalman", "KotakAmal", "salman");
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_I2C lcd(0x27,16,2);



void setup() {
  Serial.begin(9600); // kecepatan pengiriman data

      // LCD
      lcd.init();
      lcd.clear();
      lcd.backlight();
      // pesan awal LCD ketika sistem sedang berjalan
      lcd.setCursor(0,0);
      lcd.print("SISTEM  KEAMANAN");
      lcd.setCursor(0,1);
      lcd.print("KOTAK AMAL:AKTIF");

      // mode perangkat
      motorservo.attach(pinServo);
      pinMode(pinBuzzer, OUTPUT); // buzzer
      digitalWrite(pinBuzzer, LOW);
  
      // sambungan koneksi ke jaringan WiFi
      thing.SetWiFi("WiFi Lantai 1 1", "");
      thing.initDevice();
  
      delay(100);
      Serial.println("\n\nTes akses masuk menggunakan fingerprint");
      finger.begin(57600);
    
      if (finger.verifyPassword()) {
        Serial.println("Found fingerprint sensor!");
          
      } else {
        Serial.println("Did not find fingerprint sensor :(");
        while (1) { delay(1); }
      }
   
      pinMode(ECHOPIN, INPUT); // sensor ultrasonik
      pinMode(TRIGPIN, OUTPUT); // sensor ultrasonik
      motorservo.write(0);
     
    
}

void loop() {
      digitalWrite(pinBuzzer, LOW);

      // proses kerja sensor sidik jari
      getFingerprintID();
      delay(500);
      thing.Handle();
      kirimPesan(jarak);
      // proses kerja sensor ultrasonik
      digitalWrite(TRIGPIN, LOW);             
      delayMicroseconds(2);
      digitalWrite(TRIGPIN, HIGH);                  
      delayMicroseconds(10);
      digitalWrite(TRIGPIN, LOW);
      timer = pulseIn(ECHOPIN, HIGH);
      jarak = timer*0.034/2;
      
      delay(1000);
      
      
      Serial.print("Jarak = ");
      Serial.print(jarak);
      Serial.print(" cm");
      Serial.println();

    // proses menampilkan pesan ke LCD berdasarkan hasil pindai sidik jari
      uint8_t p = finger.getImage();
        if (p == FINGERPRINT_OK) {
          p = finger.image2Tz();
          if (p == FINGERPRINT_OK) {
            p = finger.fingerSearch();
            if (p == FINGERPRINT_OK) {
              motorservo.write(180);
              String msg = "Akses masuk melalui sidik jari diterima";
              thing.sendMsg("6289652365000", msg);
              digitalWrite(pinBuzzer, HIGH);
              delay(1000);
              digitalWrite(pinBuzzer, LOW);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("   SIDIK JARI   ");
              lcd.setCursor(0, 1);
              lcd.print("    DIKENALI    ");
              delay(5000);
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("SISTEM  KEAMANAN");
              lcd.setCursor(0,1);
              lcd.print("KOTAK AMAL:AKTIF");
              delay(10000);
            }
          }
              p = finger.fingerSearch();
              if (p == FINGERPRINT_NOTFOUND) {
              motorservo.write(0);
              String msg = "Akses masuk melalui sidik jari ditolak!";
              thing.sendMsg("6289652365000", msg);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("   SIDIK JARI   ");
              lcd.setCursor(0, 1);
              lcd.print(" TIDAK  DIKENAL ");
              delay(3000);
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("SISTEM  KEAMANAN");
              lcd.setCursor(0,1);
              lcd.print("KOTAK AMAL:AKTIF");
            }
      }
      digitalWrite(pinBuzzer, LOW);
}
       

// kode fingerprint
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

// OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  }

// found a match!
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
   
    return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

//   found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
    
  
  return finger.fingerID;
}

void kirimPesan(int nilai){
  if((nilai >= 10) && (nilai <= 500)){
    digitalWrite(pinBuzzer, HIGH);
    String msg = "Kotak Amal Sedang dibawa pergi!";
    thing.sendMsg("6289652365000", msg);
//    Serial.print("Jarak = ");
//    Serial.print(jarak);
//    Serial.print(" cm");
//    Serial.println();
//    delay(10000);
    
   }
}

// kode sistem keamanan IoT
String HandleResponse(String query)
{
  
  if (query == "buzzon") {
   digitalWrite(pinBuzzer, HIGH);
//   delay(1000);
   return "Buzzer dinyalakan";   
  }

   if (query == "buzzoff") {
   digitalWrite(pinBuzzer, LOW);
//   delay(1000);
   return "Buzzer sudah dimatikan";   
  }

   if (query == "kunci") {
   motorservo.write(0); 
   digitalWrite(pinBuzzer, HIGH);
   digitalWrite(pinBuzzer, LOW);
   lcd.setCursor(0,0);
   lcd.print("SISTEM  KEAMANAN");
   lcd.setCursor(0,1);
   lcd.print("KOTAK AMAL:AKTIF");
   return "Kotak amal sudah dalam keadaan terkunci";   
  
  }

   if (query == "buka") {
   motorservo.write(180);
   digitalWrite(pinBuzzer, HIGH);
   delay(1000);
   digitalWrite(pinBuzzer, LOW);
   lcd.setCursor(0,0);
   lcd.print("SISTEM  KEAMANAN");
   lcd.setCursor(0,1);
   lcd.print("KOTAK AMAL: MATI");
   return "Kotak amal dalam keadaan terbuka";   
  }


  else return "Kata kuncinya salah";
}
