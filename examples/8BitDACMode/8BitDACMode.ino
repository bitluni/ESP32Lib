//You need to connect a VGA screen cable to the pins specified below.
//cc by-sa 4.0 license
//Martin-Laclaustra
/*
  CONNECTION
  
  A) voltageDivider = false; B) voltageDivider = true (last init parameter)
  
     55 shades                  255 shades
  
  ESP32        VGA           ESP32                       VGA     
  -----+                     -----+    ____ 100 ohm              
      G|-   +---- R              G|---|____|+         +---- R    
  pin25|----+---- G          pin25|---|____|+---------+---- G    
  pin26|-   +---- B          pin26|-        220 ohm   +---- B    
  pin X|--------- HSYNC      pin X|------------------------ HSYNC
  pin Y|--------- VSYNC      pin Y|------------------------ VSYNC
  -----+                     -----+                              
  
  Connect pin 25 or 26
  Connect the 3 channels in parallel or whatever combination of them
    depending on the monochrome color of choice
*/

#include <ESP32Video.h>
#include <Ressources/Font6x8.h>

//pin configuration
const int outputPin = 25;
const int hsyncPin = 16;
const int vsyncPin = 4;

VGA8BitDAC videodisplay;

void setup()
{
	//initializing vga at the specified pins
  //output pin and boolean for voltage divider can be omitted
  videodisplay.init(VGAMode::MODE320x240, hsyncPin, vsyncPin, outputPin, false);
	//selecting the font
	videodisplay.setFont(Font6x8);
	//displaying the test pattern
  videodisplay.rect(30, 88, 255+5, 40+4, 127);
  for(int x = 0; x < 256; x++)
  {
    videodisplay.fillRect(x + 32, 90, 1, 40, x);
    if(x % 16 == 0)
    {
      videodisplay.fillRect(x + 32, 85, 1, 4, 255);
      videodisplay.setCursor(x + 32 - 3, 78);
      videodisplay.print(x,HEX);
    }
  }
}

void loop()
{
}
