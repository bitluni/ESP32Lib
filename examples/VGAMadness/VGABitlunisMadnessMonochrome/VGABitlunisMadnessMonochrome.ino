#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>
#include "VGA/VGA6Multimonitor.h"

//pin configuration
//red pins
//const int M0Pin = 17;
//const int M1Pin = 2;
//const int M2Pin = 19;
//const int M3Pin = 12;
//const int M4Pin = 23;
//const int M5Pin = 26;
//green pins
const int M0Pin = 16;
const int M1Pin = 15;
const int M2Pin = 18;
const int M3Pin = 14;
const int M4Pin = 22;
const int M5Pin = 25;
//blue pins
//const int M0Pin = 4;
//const int M1Pin = 13;
//const int M2Pin = 5;
//const int M3Pin = 27;
//const int M4Pin = 21;
//const int M5Pin = 0;
const int hsyncPin = 32;
const int vsyncPin = 33;

//VGA Device
VGA6Multimonitor videodisplay;

void setup()
{
  Serial.begin(115200);
  //initializing vga at the specified pins
  videodisplay.init(VGAMode::MODE320x240,
      M0Pin, M1Pin,
      M2Pin, M3Pin,
      M4Pin, M5Pin,
      hsyncPin, vsyncPin, -1,3,2);
  //selecting the font
  videodisplay.setFont(Font6x8);

  for(int xmon = 0; xmon < 3; xmon++)
  {
    for(int ymon = 0; ymon < 2; ymon++)
    {
      videodisplay.line(xmon*320 + 160-14,ymon*240+120+3,xmon*320 + 160+22,ymon*240+120+3,1);
      videodisplay.line(xmon*320 + 160-2,ymon*240+120-12,xmon*320 + 160-2,ymon*240+120+18,1);
      videodisplay.line(xmon*320 + 160+10,ymon*240+120-12,xmon*320 + 160+10,ymon*240+120+18,1);
    }
  }
  
  videodisplay.setCursor(160-12,120-5);
  videodisplay.println("1!");
  videodisplay.setCursor(160+320,120-5);
  videodisplay.println("2!");
  videodisplay.setCursor(160+320+320+12,120-5);
  videodisplay.println("3!");
  videodisplay.setCursor(160-12,120+240+5);
  videodisplay.println("4!");
  videodisplay.setCursor(160+320,120+240+5);
  videodisplay.println("5!");
  videodisplay.setCursor(160+320+320+12,120+240+5);
  videodisplay.println("6!");
}


void loop() {
}
