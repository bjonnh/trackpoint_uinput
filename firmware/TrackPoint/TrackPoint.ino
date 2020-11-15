#include "TrackPoint.h"

// Make sure you change these to the pins you use
#define CLOCK       4
#define DATA        3
#define RESET       2

// You don't need to change that unless you are using the interruptions for something else
#define CLOCK_INT   0

TrackPoint trackpoint(CLOCK, DATA, RESET, true);

String inputString = "";
bool stringComplete = false;

void setup()
{
  inputString.reserve(200);
  Serial.begin(115200);
  trackpoint.reset();
  trackpoint.setSensitivityFactor(0xb0);
  trackpoint.setRemoteMode();
  attachInterrupt(CLOCK_INT, clockInterrupt, FALLING);
}

int btnStatus = 0;

void loop()
{
  if (stringComplete) {
    inputString.trim();
    if (inputString == "WHO") {
      Serial.println("ME");
    } else if (inputString == "RESET") {
      Serial.println("OK");
      trackpoint.reset();
      trackpoint.setSensitivityFactor(0xb0);
      trackpoint.setRemoteMode();
      Serial.println("READY");
    }
    inputString = "";
    stringComplete = false;
  }
  
  TrackPoint::DataReport d = trackpoint.readData();
  if ((d.x!=0) ||  (d.y!=0)) {
    Serial.print("m ");
    Serial.print(d.x);
    Serial.print(" ");
    Serial.println(-d.y);
  }
   
  for (int btn=1;btn<4;btn++) {
    int pos = (1<<(btn-1));
    if (((d.state&pos) != 0) && (btnStatus&pos) == 0) {
      Serial.print("c ");
      Serial.println(btn);
      btnStatus = btnStatus | pos;
    }
    if (((d.state&pos) == 0) && (btnStatus&pos) != 0) {
      Serial.print("r ");
      Serial.println(btn);
      btnStatus = btnStatus & (0b1111 - pos);
    }
  }
}

void clockInterrupt(void) {
  trackpoint.getDataBit();
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
