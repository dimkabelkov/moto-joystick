
#include  <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(7, 8);
const uint64_t pipe = 0xE8E8F0F0E1LL;

const int switchPin = 9;
const int pinX = A2;
const int pinY = A1;

bool isRun = false;
bool lasuUp = true;

int lastX = 0;
int lastY = 0;

int dX = 0;
int dY = 0;

int deltaTime = 1000;

void setup() {

//  Serial.begin(115200);

  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(switchPin, INPUT);
  digitalWrite(switchPin, HIGH);

  radio.begin();
  radio.setChannel(2);
  radio.setPayloadSize(5);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipe);

//  Serial.println("Setup ...");
}

void loop() {
  int X = analogRead(pinX) - 512;
  int Y = analogRead(pinY) - 512;

  if (deltaTime) {
    dX = X;
    dY = Y;

    deltaTime --;
    if (!deltaTime) {
//      Serial.println("dX: " + String(dX));
//      Serial.println("dY: " + String(dY));
    }
  }

  byte bytes[5] = {0, 0, 0, 0, 0};

  bool switchState = !digitalRead(switchPin);
  if (switchState && !isRun && lasuUp) {
    isRun = true;
    lasuUp = false;
    bytes[0] = 1;
    radio.write(bytes, 5);
//    Serial.println("Start ...");
  } else if (switchState && isRun && lasuUp) {
    isRun = false;
    lasuUp = false;
    radio.write(bytes, 5);
//    Serial.println("Stop ...");
  } else if (isRun) {
    X -= dX;
    Y -= dY;
    bytes[0] = 2;
    if (abs(X - lastX) > 2 || abs(Y - lastY) > 2) {
//      Serial.println("     X: " + String(X) + ",      Y: " + String(Y));
      int bX = abs(X);
      int bIndex = 2;
      while (bX) {
        bytes[bIndex] = bX & 255;
        bX = bX >> 8;
        bIndex--;
      }
      if (X > 0) {
        bytes[1] = bytes[1] | B00000100;
      }

      int bY = abs(Y);
      bIndex = 4;
      while (bY) {
        bytes[bIndex] = bY & 255;
        bY = bY >> 8;
        bIndex--;
      }
      if (Y > 0) {
        bytes[3] = bytes[3] | B00000100;
      }
      radio.write(bytes, 5);

      int checkX = 0;
      int checkY = 0;

      checkX = bytes[1] & ~ B00000100;
      checkX = checkX << 8 | bytes[2];
      if (!(bytes[1] & B00000100)) {
        checkX = checkX * -1;
      }

      checkY = bytes[3] & ~ B00000100;
      checkY = checkY << 8 | bytes[4];

      if (!(bytes[3] & B00000100)) {
        checkY = checkY * -1;
      }

//      Serial.println("checkX: " + String(checkX) + ", checkY: " + String(checkY));

      lastX = X;
      lastY = Y;
    }
  }
  if (!switchState) {
    lasuUp = true;
  }
}












