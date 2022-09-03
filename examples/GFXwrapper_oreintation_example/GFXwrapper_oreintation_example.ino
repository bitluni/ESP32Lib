/* 
 * Date  : 7/8/2022
 * Author: Md. Asifuzzaman Khan
 * Github: https://github.com/AsifKhan991/Customized-GFXwrapper-library-for-Bitluni-ESP32
 * 
 * This example shows how the display orientation can be changed using Bitluni's ESP32 VGA library by 
 * the "object.orietnation" vairable of  Gfxwrapper class.
 * 
 * Tested on ESP32 DevKit-V1 & DevKitC-V4
 * 
 * There are 4 modes total:
 * object.orientation = 0   -> (default)actual orientation 
 * object.orientation = 90  -> 90 degree clockwise rotation relative to actual
 * object.orientation = 180 -> 180 degree clockwise rotation relative to actual
 * object.orientation = 270 -> 270 degree clockwise rotation relative to actual
 * object.orientation = -1  -> flip horizontally
 * object.orientation = -2  -> flip vertically
 * 
 * Credit goes to bitluni for creating such an amazing library.
 * Project page: https://github.com/bitluni/ESP32Lib
 */
#include <ESP32Lib.h>
#include <GfxWrapper.h>
#include <Fonts/FreeSansBold12pt7b.h>

const int redPin = 14;
const int greenPin = 12;
const int bluePin = 27;
const int hsyncPin = 32;
const int vsyncPin = 33;
VGA3BitI vga;
GfxWrapper<VGA3BitI> gfx(vga, 640, 480); 

void setup() {
  vga.init(vga.MODE640x480, redPin, greenPin, bluePin, hsyncPin, vsyncPin); // initiating in 640 by 480 resolution
  gfx.setTextColor(0x0F00);
  gfx.setFont(&FreeSansBold12pt7b);
}

void loop() {
  gfx.orientation = 0; //Not necessary to declare the default orientation
  gfx.setCursor(80 , 40);
  gfx.print("Rotation example");
  gfx.orientation = 90;
  gfx.setCursor(80 , 40);
  gfx.print("Rotation example");
  gfx.orientation = 180;
  gfx.setCursor(80 , 40);
  gfx.print("Rotation example");
  gfx.orientation = 270;
  gfx.setCursor(80 , 40);
  gfx.print("Rotation example");

  gfx.orientation = 0;
  gfx.setCursor(325 , 150);
  gfx.print("Horizontal flip");
  gfx.orientation = -1;
  gfx.setCursor(325 , 150);
  gfx.print("Horizontal flip");

  gfx.orientation = 0;
  gfx.setCursor(220 , 235);
  gfx.print("Vertical flip");
  gfx.orientation = -2;
  gfx.setCursor(220  , 235);
  gfx.print("Vertical flip");

}
