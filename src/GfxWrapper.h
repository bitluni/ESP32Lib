/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once
#include "Adafruit_GFX.h"


template<class Base>
class GfxWrapper : public Adafruit_GFX
{ 
  public:
	int Xres,Yres,orientation=0;
	Base &base;
	typedef typename Base::Color Color;
	GfxWrapper(Base &vga,const int  xres, const int yres)
		:base(vga),
		Adafruit_GFX(xres, yres)
	{
		Xres=xres;
		Yres=yres;
	}

	virtual void drawPixel(int16_t x, int16_t y, uint16_t color)
	{	
		if(orientation==0){
			base.dot(x, y, base.RGBA((color >> 8) & 0b11111000, (color >> 3) & 0b11111100, (color << 3) & 0b11111000)); // actual orientation
		}else if(orientation==90){
			base.dot(Xres-y, x, base.RGBA((color >> 8) & 0b11111000, (color >> 3) & 0b11111100, (color << 3) & 0b11111000)); // 90 degree c.c rotation from actual
		}else if(orientation==180){
			base.dot(Xres-x, Yres-y, base.RGBA((color >> 8) & 0b11111000, (color >> 3) & 0b11111100, (color << 3) & 0b11111000)); //180 degree c.c rotation from actual
		}else if(orientation==270){
			base.dot(y, Yres-x, base.RGBA((color >> 8) & 0b11111000, (color >> 3) & 0b11111100, (color << 3) & 0b11111000)); //270 degree c.c rotation from actual
		}else if(orientation==-1){
			base.dot(Xres-x, y, base.RGBA((color >> 8) & 0b11111000, (color >> 3) & 0b11111100, (color << 3) & 0b11111000)); //horizontal flip
		}else if(orientation==-2){
			base.dot(x, Yres-y, base.RGBA((color >> 8) & 0b11111000, (color >> 3) & 0b11111100, (color << 3) & 0b11111000)); //vertical flip
		}
		
	}
};
