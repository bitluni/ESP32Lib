//You need to connect a VGA screen cable and an external DAC (simple R2R does the job) to the pins specified below.
//cc by-sa 2.0 license
//bitluni

//include libraries
#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>

//pin configuration
const int redPins[] = {2, 4, 12, 13, 14};
const int greenPins[] = {15, 16, 17, 18, 19};
const int bluePins[] = {21, 22, 23, 27};
const int hsyncPin = 0;
const int vsyncPin = 5;

//VGA Device
VGA14Bit vga;


//just draw each frame
void loop()
{
  //clear the back buffer  
  vga.clear(0);

  vga.show();
}