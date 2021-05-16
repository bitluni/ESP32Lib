//This example shows a simple "Hello world!" on a VGA screen.
//You need to connect a VGA screen cable to the pins specified below.
//cc by-sa 4.0 license
//bitluni

#include <ESP32Video.h>
#include <Ressources/Font6x8.h>
#include "VGA/VGA4ColorMultimonitor.h"

//pin configuration

//red pins
const int redPin = 17;
const int redPin2 = 2;
const int redPin3 = 19;
const int redPin4 = 12;
//const int M4Pin = 23;
//const int M5Pin = 26;
//green pins
const int greenPin = 16;
const int greenPin2 = 15;
const int greenPin3 = 18;
const int greenPin4 = 14;
//const int M4Pin = 22;
//const int M5Pin = 25;
//blue pins
const int bluePin = 4;
const int bluePin2 = 13;
const int bluePin3 = 5;
const int bluePin4 = 27;
//const int M4Pin = 21;
//const int M5Pin = 0;
const int hsyncPin = 32;
const int vsyncPin = 33;

//VGA Device
VGA4ColorMultimonitor videodisplay;

void setup()
{
	//initializing vga at the specified pins
  videodisplay.init(VGAMode::MODE320x240, redPin, greenPin, bluePin,
                                          redPin2, greenPin2, bluePin2,
                                          redPin3, greenPin3, bluePin3,
                                          redPin4, greenPin4, bluePin4,
                                          -1, -1, hsyncPin, vsyncPin, -1,2,2);
	//selecting the font
	videodisplay.setFont(Font6x8);
	//displaying the text
  videodisplay.fillCircle(140,10,5,1);
  videodisplay.fillCircle(140+20+videodisplay.mode.hRes,10,5,1);
  videodisplay.fillCircle(140+20*2+videodisplay.mode.hRes*2,10,5,1);
  videodisplay.fillCircle(140,30+videodisplay.mode.vRes,5,1);
  videodisplay.fillCircle(140+20+videodisplay.mode.hRes,30+videodisplay.mode.vRes,5,1);
  videodisplay.fillCircle(140+20*2+videodisplay.mode.hRes*2,30+videodisplay.mode.vRes,5,1);
  videodisplay.fillCircle(videodisplay.mode.hRes/2,videodisplay.mode.vRes/videodisplay.mode.vDiv,80,videodisplay.RGB(255,255,0));
  videodisplay.fillCircle(videodisplay.mode.hRes,videodisplay.mode.vRes/videodisplay.mode.vDiv/2,80,videodisplay.RGB(255,0,255));

  videodisplay.setFont(Font6x8);

  for(int xmon = 0; xmon < 3; xmon++)
  {
    for(int ymon = 0; ymon < 2; ymon++)
    {
      videodisplay.line(xmon*320 + 160-14,ymon*240+120+3,xmon*320 + 160+22,ymon*240+120+3,videodisplay.RGB(0,255,255));
      videodisplay.line(xmon*320 + 160-2,ymon*240+120-12,xmon*320 + 160-2,ymon*240+120+18,videodisplay.RGB(0,255,0));
    }
  }
  
  videodisplay.setCursor(160-12,120-5);
  videodisplay.println("1!");
  videodisplay.setCursor(160+320,120-5);
  videodisplay.println("2!");
  videodisplay.setCursor(160-12,120+240+5);
  videodisplay.println("3!");
  videodisplay.setCursor(160+320,120+240+5);
  videodisplay.println("4!");

}

void loop()
{
}
