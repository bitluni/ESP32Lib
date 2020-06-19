//include libraries
#include <ESP32Lib.h>

//include the sprites converted the SpriteConverter. It's an html found in the utilities folder of the library
#include "nyan.h"

//To know where all the pixels are you need to implement a class with the function map like here:
class LEDWall3: public ParallelLEDGraphics
{
  public:
  LEDWall3()
  :ParallelLEDGraphics(48, 40, 4)
  {
  }

  //this function maps from x,y to channel and led number. channel, and led are return parameters
  virtual bool map(int x, int y, int &channel, int &led)
  {
    //no checks in this example needed
    //if(x < 0 || x >= 48 || y < 0 || y >= 40) 
    //return false;
    channel = x >> 3;
    led = (x & 7) + y * 8;
    return true;
  }
};

LEDWall3 gfx;

//initial setup
void setup()
{
  const int pins[] = {16, 12, 13, 15, 14, 2, -1, -1};
  gfx.init(pins, 8 * 40);
  gfx.setGamma(3.2f, 2.0f, 4.0f, 3.2f); //color adjustment for LED displays
}

//just draw each frame
void loop()
{
  static int i = 0; 
  i = (i + 1) % 6;  //circulate the 6 sprites
	nyan.draw(gfx, i, 24, 20);
	gfx.show();
  delay(50);
}
