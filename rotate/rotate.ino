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

int motor1Speed1 = 5; 
int motor1Speed2 = 18;
int motor1SpeedPwm = 23;

int speed = 0;
int speedNew = 0;

byte relayPin = 19;

bool isRun = false;

bool isDebug = false;

const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

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

  pinMode(motor1Speed1, OUTPUT);
  pinMode(motor1Speed2, OUTPUT);
  pinMode(motor1SpeedPwm, OUTPUT);

  pinMode(relayPin, OUTPUT);

  ledcSetup(pwmChannel, freq, resolution);
  
  ledcAttachPin(motor1SpeedPwm, pwmChannel);

  radio.begin();
  radio.setChannel(2);
  radio.setPayloadSize(5);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1,pipe);
  radio.startListening();
}

void loop()
{
//  if (Serial.available() && !radio.available()) {
//    if (!commandLength) {
//      readSerial = true;
//    }
//    command[commandLength++] = Serial.read();
//    iterateSerial = 300;
//  } else if (radio.available() && !Serial.available()) {
  if (radio.available()) {
      String xData = "";
      bool isX = false;
      byte joystickData[5];
      radio.read(joystickData, 5);

      if (isRun) {
        int checkX = 0;
        int checkY = 0;
        
        checkX = joystickData[1] &~ B00000100;
        checkX = checkX << 8 | joystickData[2];
        if (!(joystickData[1] & B00000100)) {
          checkX = checkX * -1;
        }
  
        checkY = joystickData[3] &~ B00000100;
        checkY = checkY << 8 | joystickData[4];
  
        if (!(joystickData[3] & B00000100)) {
          checkY = checkY * -1;
        }
        
        if (isDebug) {
          Serial.println("checkX: " + String(checkX) + ", checkY: " + String(checkY));
        }

        rotateNew = checkX;
        speedNew = checkY;
          
      } else if (rotateNew != 0 || speedNew != 0) {
        rotateNew = 0;
        speedNew = 0;
      }

      if (rotate != rotateNew && isDebug) {
        Serial.println("rotateNew: " + String(rotateNew));
      }

      if (speed != speedNew && isDebug) {
        Serial.println("speedNew: " + String(speedNew));
      }

      if (joystickData[0] == 1) {
        isRun = true;
        if (isDebug) {
          Serial.println("Start...");
        }
        digitalWrite (relayPin, HIGH); 
      } else if (joystickData[0] == 0) {
        isRun = false;
        if (isDebug) {
          Serial.println("Stop..."); 
        }
        digitalWrite (relayPin, LOW);
      }
      
  } else if (readSerial && !(iterateSerial--)) {
//    readSerial = false;
//    String xData = "";
//    bool isX = false;
//    for(byte i = 0; i < commandLength; i++)  {
//      if (isX) {
//        xData += char(command[i]);
//      } else if (String(char(command[i])) == ":") {
//        isX = true;
//      }
//    }
//    Serial.println("Serial: " + xData);
//    rotateNew = xData.toInt() * -1;
//    commandLength = 0;
  }

  rotateHandler();
  speedHandler();
}

void speedHandler()
{
  if (abs(speedNew) < 15) {
    digitalWrite (motor1Speed1, LOW);
    digitalWrite (motor1Speed2, LOW); 
    ledcWrite(pwmChannel, 0);
  } else if (speed != speedNew) {
    speed = speedNew;
    int speedMath = abs(speed) - 15;
    if (speedMath > 490) {
      speedMath = 490;
    }
    if (isDebug) {
      Serial.println("speed PWM: " + String(150 + speedMath / 4.7));
    }
    ledcWrite(pwmChannel, 150 + speedMath / 4.7);
    digitalWrite (motor1Speed1, speed > 0 ? HIGH : LOW);
    digitalWrite (motor1Speed2, speed > 0 ? LOW : HIGH);
  }
}

void rotateHandler()
{
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

