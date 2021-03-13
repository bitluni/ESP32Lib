//This example shows a simple "Hello world!" on a VGA screen.
//You need to connect a VGA screen cable to the pins specified below.
//cc by-sa 4.0 license
//bitluni

#include <ESP32Video.h>
#include <Ressources/Font6x8.h>

//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 32;
const int vsyncPin = 33;

//VGA Device
VGA1BitI videodisplay;

const uint8_t frontColors[] = {0x2,0x0,0x1,0x4,0x1,0x7,0x3};
const uint8_t backColors[] = {0x0,0x7,0x0,0x6,0x7,0x0,0x4};

void setup()
{
	//initializing vga at the specified pins
	videodisplay.init(VGAMode::MODE320x240, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
	//selecting the font
	videodisplay.setFont(Font6x8);
  videodisplay.setCursor(120,115);
  //displaying the text
	//default coloring
	videodisplay.println("Hello World!");
	delay(10000);
	//custom coloring
  for(int i = 0; i < 10; i++)
  {
    videodisplay.setFrontGlobalColor(0,0,255);
    videodisplay.setBackGlobalColor(0,0,0);
    delay(500);
  	videodisplay.setFrontGlobalColor(255,255,0);
  	videodisplay.setBackGlobalColor(0,0,255);
    delay(500);
  }
  delay(2000);
	videodisplay.clear();
}

void loop()
{
	for(int i = 0; i < 10; i++)
	{
		videodisplay.setCursor(random(1,300),random(1,220));
		videodisplay.println("Hello World!");
		delay(250);
	}
	delay(1500);
	videodisplay.clear();
	static int currentpalette = 0;
	videodisplay.frontGlobalColor = frontColors[currentpalette];
	videodisplay.backGlobalColor = backColors[currentpalette];
	currentpalette = (currentpalette + 1)%sizeof(frontColors);
}
