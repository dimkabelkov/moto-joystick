#include  <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(7,8);
const uint64_t pipe = 0xE8E8F0F0E1LL;

const int switchPin = 9;
const int pinX = A1;
const int pinY = A2;

bool isRun = false;
bool lasuUp = true;

int lastX = 0;
int lastY = 0;
 
void setup() {

  Serial.begin(115200);
  
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(switchPin, INPUT);
  digitalWrite(switchPin, HIGH);

  radio.begin();
  radio.setChannel(2);
  radio.setPayloadSize(12);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipe);
}
 
void loop() {
  bool switchState = !digitalRead(switchPin);
  if (switchState && !isRun && lasuUp) {
    isRun = true;
    lasuUp = false;
  } else if (switchState && isRun && lasuUp) {
    isRun = false;
    lasuUp = false;
  } else if (isRun) {
    int X = analogRead(pinX) - 512; // считываем аналоговое значение оси Х
    int Y = analogRead(pinY) - 512; // считываем аналоговое значение оси Y

    int diffX = abs(X - lastX);
    int diffY = abs(Y - lastY);
    
    if (diffX > 2 || diffY > 2) {
      lastX = X;
      lastY = Y;

      char copy[11];
      
      String send = "";
      send = ">" + String(X) + ":" + String(Y) + "<";

      send.toCharArray(copy, send.length());
      Serial.println(send + "," + String(send.length()));
      radio.write(copy, send.length());
    }
  }
  if (!switchState) {
    lasuUp = true;
  }
}












