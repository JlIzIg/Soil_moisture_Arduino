#include "DFRobot_RGBLCD1602.h"
#include "AESLib.h"
#include "arduino_base64.hpp"


const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

DFRobot_RGBLCD1602 lcd(16, 2);
int hum = 0;
int boundaryMin = 25;
int boundaryMax = 75;

const int ledPin1 = 4;
const int ledPin2 = 5;
/*
unsigned long p;
unsigned long g;*/
unsigned long p = 47;       // Prime number shared between both Arduinos
unsigned long g = 5;        // Primitive root shared between both Arduinos
unsigned long privateKeyB;  // Private key of Arduino B
unsigned long publicKeyB;   // Public key of Arduino B
unsigned long secretKey;    // Shared secret key
unsigned long publicKeyA;   // Public key for Arduino A


AESLib aesLib;

byte extendedKey[16];

// Функция быстрого возведения в степень по модулю
unsigned long modPow(unsigned long base, unsigned long exponent, unsigned long modulus) {
  unsigned long result = 1;
  while (exponent > 0) {
    if (exponent % 2 == 1) {
      result = (result * base) % modulus;
    }
    base = (base * base) % modulus;
    exponent /= 2;
  }
  return result;
}

// the decryption function
String decrypt(String text, int secretKey) {


  memset(extendedKey, 0, sizeof(extendedKey));
  memcpy(extendedKey, &secretKey, sizeof(secretKey));

  int origBytesLenght = base64::decodeLength(text.c_str());

  byte encryptedBytes[origBytesLenght];
  byte decryptedBytes[origBytesLenght];

  base64::decode(text.c_str(), encryptedBytes);
  byte aesIv[] = { 123, 43, 46, 89, 29, 187, 58, 213, 78, 50, 19, 106, 205, 1, 5, 7 };
  aesLib.set_paddingmode((paddingMode)0);

  // розшифрування
  aesLib.decrypt(encryptedBytes, origBytesLenght, decryptedBytes, extendedKey, 16, aesIv);

  String origText = String((char*)decryptedBytes);

  return origText;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(50);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, HIGH);
  lcd.init();
  lcd.setRGB(colorR, colorG, colorB);
  lcd.noDisplay();
}



void loop() {
  /*while (!Serial.available());
  {
    p = Serial.parseInt();

    g=Serial.parseInt();
  }*/
  /*Serial.end();
  Serial.begin(9600);*/

  privateKeyB = random(1, p);
  publicKeyB = modPow(g, privateKeyB, p);

  while (!Serial.available());
  {
    publicKeyA = Serial.parseInt();
  }
  Serial.println(publicKeyB);
  Serial.end();
  Serial.begin(9600);

  lcd.display();
  secretKey = modPow(publicKeyA, privateKeyB, p);
  Serial.end();
  Serial.begin(9600);

  String humdt=Serial.readStringUntil('\n');
  int hum = decrypt(humdt, secretKey).toInt();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("Humidity:");
  lcd.setCursor(0, 1);
  lcd.println(String(hum) + " %");

  if (hum < boundaryMin) {
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, HIGH);
  }
  if (hum > boundaryMax) {
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
  }
  if (hum > boundaryMin && hum < boundaryMax) {
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, HIGH);
  }
}
