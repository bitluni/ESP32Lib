//This example shows how to use the GfxWrapper to be able to use the Adafruit GFX library with VGA
//cc by-sa 4.0 license
//bitluni

#include <ESP32Video.h>
#include <Ressources/Font6x8.h>
#include <GfxWrapper.h>
#include <Fonts/FreeMonoBoldOblique24pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>

//VGA Device
VGA6Bit videodisplay;
GfxWrapper<VGA6Bit> gfx(videodisplay, 640, 400);

//initial setup
void setup()
{
	//initializing i2s vga (with only one framebuffer)
  //Pin presets are avaialable for: VGAv01, VGABlackEdition, VGAWhiteEdition, PicoVGA
  //But you can also use custom pins. Check the other examples
	videodisplay.init(VGAMode::MODE640x400, videodisplay.VGABlackEdition);
	//using adafruit gfx
  gfx.setFont(&FreeMonoBoldOblique24pt7b);
  gfx.setCursor(100, 100);
  gfx.print("Hello");
}

//the loop is done every frame
void loop()
{
  
}
