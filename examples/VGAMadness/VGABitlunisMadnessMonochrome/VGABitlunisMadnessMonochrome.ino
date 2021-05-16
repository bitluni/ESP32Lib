#include <ESP32Video.h>
#include <Ressources/Font6x8.h>
#include "VGA/VGA6MonochromeVGAMadnessMultimonitor.h"

//pin configuration
//red pins
const int redPin0 = 17;
const int redPin1 = 2;
const int redPin2 = 19;
const int redPin3 = 12;
const int redPin4 = 23;
const int redPin5 = 26;
//green pins
const int greenPin0 = 16;
const int greenPin1 = 15;
const int greenPin2 = 18;
const int greenPin3 = 14;
const int greenPin4 = 22;
const int greenPin5 = 25;
//blue pins
const int bluePin0 = 4;
const int bluePin1 = 13;
const int bluePin2 = 5;
const int bluePin3 = 27;
const int bluePin4 = 21;
const int bluePin5 = 0;
const int hsyncPin = 32;
const int vsyncPin = 33;

//VGA Device
VGA6MonochromeVGAMadnessMultimonitor videodisplay;

void setup()
{
  Serial.begin(115200);
  //selecting the foreground or background color
  //IMPORTANT: for monochrome mode this MUST be done BEFORE init
  //           and color can not be changed after init
  //uncomment the line to test changing foreground color in all outputs
  //videodisplay.setFrontGlobalColor(255,255,0); //yellow
  //uncomment the line to test changing background color in all outputs
  //videodisplay.setBackGlobalColor(255,0,0); //red
  //because of hardware limitations only BackGlobalColors compatible with FrontGlobalColor will set a non-black background:
  // FrontGlobalColor - cyan, BackGlobalColors - green or blue
  // FrontGlobalColor - magenta, BackGlobalColors - red or blue
  // FrontGlobalColor - yellow, BackGlobalColors - red or green
  // FrontGlobalColor - white, BackGlobalColors - any
  
  //initializing vga at the specified pins
  videodisplay.init(VGAMode::MODE320x240,
                    redPin0,greenPin0,bluePin0,
                    redPin1,greenPin1,bluePin1,
                    redPin2,greenPin2,bluePin2,
                    redPin3,greenPin3,bluePin3,
                    redPin4,greenPin4,bluePin4,
                    redPin5,greenPin5,bluePin5,
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
