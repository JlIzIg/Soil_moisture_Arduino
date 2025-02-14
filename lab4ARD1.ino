#include <Arduino.h>
#include "DFRobot_RGBLCD1602.h"
#include "AESLib.h"
#include "arduino_base64.hpp"

const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

DFRobot_RGBLCD1602 lcd(16, 2);
int aPin = A0;
int flag = 0;
int avalue = 0;
const int buttonPin = 2;

/*unsigned long p;
unsigned long g;*/
unsigned long p = 47;       // Prime number shared between both Arduinos
unsigned long g = 5;       // Primitive root shared between both Arduinos
unsigned long privateKeyA;  // Private key of Arduino A
unsigned long publicKeyA;   // Public key of Arduino A
unsigned long secretKey;    // Shared secret key
unsigned long publicKeyB;   // Public key for Arduino B

AESLib aesLib;

byte extendedKey[16];
// Функция генерации случайного простого числа в заданном диапазоне
unsigned long generateRandomPrime(unsigned long min, unsigned long max) {
  unsigned long prime = random(min, max);
  while (!isPrime(prime)) {
    prime = random(min, max);
  }
  return prime;
}

// Функция поиска первообразного корня по модулю p
unsigned long findPrimitiveRoot(unsigned long p) {
  // Перебираем все числа от 2 до p-1
  for (unsigned long g = 2; g < p; ++g) {
    bool isPrimitive = true;
    // Проверяем, является ли число g первообразным корнем
    for (unsigned long i = 1; i < p - 1; ++i) {
      if (modPow(g, i, p) == 1) {
        isPrimitive = false;
        break;
      }
    }
    if (isPrimitive) {
      return g;
    }
  }
  return 0;  // Если не удалось найти первообразный корень
}

// Функция проверки, является ли число простым
bool isPrime(unsigned long n) {
  if (n <= 1) {
    return false;
  }
  for (unsigned long i = 2; i <= sqrt(n); ++i) {
    if (n % i == 0) {
      return false;
    }
  }
  return true;
}

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


String encrypt(String text, int secretKey) {
  // довжина байтів вхідного тексту
  int bytesInputLength = text.length() + 1;
  // масив для збереження тексту переведеного у байти
  byte bytesInput[bytesInputLength];
  // переведення вхідного повідомлення у байти
  text.getBytes(bytesInput, bytesInputLength);
  // довжина байтів після шифрування
  int outputLength = aesLib.get_cipher_length(bytesInputLength);
  // байтовий масив для заповнення результату шифруванння
  byte bytesEncrypted[outputLength];
  /*Cipher Mode = CBC
    Key Size = 128*/
  memset(extendedKey, 0, sizeof(extendedKey));
  memcpy(extendedKey, &secretKey, sizeof(secretKey));
  byte aesIv[] = { 123, 43, 46, 89, 29, 187, 58, 213, 78, 50, 19, 106, 205, 1, 5, 7 };

  // padding mode - CMS
  aesLib.set_paddingmode((paddingMode)0);
  // шифрування
  aesLib.encrypt(bytesInput, bytesInputLength, bytesEncrypted, extendedKey, 16, aesIv);

  // переведення байтів у символи
  char cypher[base64::encodeLength(outputLength)];
  base64::encode(bytesEncrypted, outputLength, cypher);

  return String(cypher);
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(50);
  pinMode(buttonPin, INPUT_PULLUP);
  lcd.init();
  lcd.setRGB(colorR, colorG, colorB);
  randomSeed(analogRead(0));  // Инициализация генератора случайных чисел

  // put your setup code here, to run once:
}

void loop() {


  if (digitalRead(buttonPin) == HIGH && flag == 0) {
    flag = 1;
  }
  if (digitalRead(buttonPin) == LOW && flag == 1) {
    flag = 0;
    avalue = analogRead(aPin);
    avalue = map(avalue, 0, 1023, 0, 100);

    /*p=generateRandomPrime(2, 100);

    g=findPrimitiveRoot(p);*/

    privateKeyA = random(1, p);
    // Calculate public key
    publicKeyA = modPow(g, privateKeyA, p);

    // Send public key to Arduino B
   /* Serial.println(p);

    Serial.println(g);*/

    Serial.println(publicKeyA);

    // Wait for Arduino B's public key
    while (!Serial.available());
    { 
      publicKeyB = Serial.parseInt(); 
    }
    Serial.end();

    secretKey = modPow(publicKeyB, privateKeyA, p);
    String Encrypted = encrypt(String(avalue), secretKey);

    Serial.begin(9600);
    Serial.println(Encrypted);

  }
}
