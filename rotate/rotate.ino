const int motorRotatePin[4] = {15, 2, 4, 16};

const bool motorRotatePosition[8][4] = {
  { 1, 0, 0, 0},
  { 1, 1, 0, 0},
  { 0, 1, 0, 0},
  { 0, 1, 1, 0},
  { 0, 0, 1, 0},
  { 0, 0, 1, 1},
  { 0, 0, 0, 1},
  { 1, 0, 0, 1}
};
byte motorRotatePositionSize = 8;

byte motorRotatePositionIndex = 0;

int rotate = 0;
int rotateNew = 0;

int iterateSerial = 0;
bool readSerial = false;
byte commandLength = 0;
byte command[128];

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(12, 14, 26, 25, 27);
const uint64_t pipe = 0xE8E8F0F0E1LL;


void setup()
{
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) {
    pinMode(motorRotatePin[i], OUTPUT);
    digitalWrite(motorRotatePin[i], LOW);
  }

 radio.begin();
 radio.setChannel(2);
 radio.setPayloadSize(12);
 radio.setDataRate(RF24_250KBPS);
 radio.openReadingPipe(1,pipe);
 radio.startListening();
}

void loop()
{
  if (Serial.available() && !radio.available()) {
    if (!commandLength) {
      readSerial = true;
    }
    command[commandLength++] = Serial.read();
    iterateSerial = 300;
  } else if (radio.available() && !Serial.available()) {
      String xData = "";
      bool isX = false;
      byte joystickData[12];
      radio.read(joystickData, 12);      
      for(int i = 1; i < 12; i++) {
        if (joystickData[i] != 0) {
          if (isX) {
            xData += char(joystickData[i]);
          } else if (String(char(joystickData[i])) == ":") {
            isX = true;
          }
        }
      }
      Serial.println("NRF: " + xData);
      rotateNew = xData.toInt() * -1;
  } else if (readSerial && !(iterateSerial--)) {
    readSerial = false;
    String xData = "";
    bool isX = false;
    for(byte i = 0; i < commandLength; i++)  {
      if (isX) {
        xData += char(command[i]);
      } else if (String(char(command[i])) == ":") {
        isX = true;
      }
    }
    Serial.println("Serial: " + xData);
    rotateNew = xData.toInt() * -1;
    commandLength = 0;
  }

  rotateHandler();
}

void rotateHandler()
{
  //Serial.println("rotate: " + String(rotate));
  //Serial.println("rotateNew: " + String(rotateNew));
  if (rotate != rotateNew) {
    if (rotateNew - rotate > 0) {
      rotate ++;
      if (motorRotatePositionIndex == motorRotatePositionSize - 1) {
        motorRotatePositionIndex = 0;
      } else {
        motorRotatePositionIndex ++;
      }
    } else {
      rotate --;
      if (motorRotatePositionIndex == 0) {
        motorRotatePositionIndex = motorRotatePositionSize - 1;
      } else {
        motorRotatePositionIndex --;
      }
    }
    rotateByPosition();
  } else {
    for (int i = 0; i < 4; i++) {
      digitalWrite(motorRotatePin[i], LOW);
    }
  }
}


void rotateByPosition()
{
  for (int i = 0; i < 4; i++) {
    digitalWrite(motorRotatePin[i], motorRotatePosition[motorRotatePositionIndex][i] ? HIGH : LOW);
  }
  delay(1);
}

