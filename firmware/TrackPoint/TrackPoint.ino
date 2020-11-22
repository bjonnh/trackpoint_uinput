#include "TrackPoint.h"
#include <Mouse.h>

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

  Mouse.begin();
  Serial.println("Setup successful");
}

void loop()
{
  serialEvent(); // serialEvent is not in the trinket M0 loop… so we have to add it manually
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
    Mouse.move(d.x,-d.y,0);
  }
  
  if ((d.state&4)!=0) {  // I inverted 3 and 1…
    if (!Mouse.isPressed(MOUSE_LEFT)) {
      Mouse.press(MOUSE_LEFT);
    }
  } else { Mouse.release(MOUSE_LEFT); }
  if ((d.state&2)!=0) {
    if (!Mouse.isPressed(MOUSE_RIGHT)) {
      Mouse.press(MOUSE_RIGHT);
    } 
  } else { Mouse.release(MOUSE_RIGHT); }
}

void clockInterrupt(void) {
  trackpoint.getDataBit();
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
   
    inputString += inChar;
    
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
