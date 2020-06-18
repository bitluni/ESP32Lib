/*
	Author: bitluni 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once

#include "ParallelLED.h"
#include "../Graphics/Graphics.h"

class ParallelLEDGraphics : public ParallelLED, public Graphics<unsigned long>
{
  public:
  	typedef unsigned long Color;
	ParallelLEDGraphics(const int xres, const int yres, int components = 3);
	virtual ~ParallelLEDGraphics();
	//graphics implementations
	virtual void show(bool vSync = false);
	
	virtual int R(Color c) const;
	virtual int G(Color c) const;
	virtual int B(Color c) const;
	virtual int A(Color c) const;
	virtual Color RGBA(int r, int g, int b, int a = 0) const;
	virtual Color RGB(int r, int g, int b) const
	{
		return RGBA(r, g, b, 0);
	}
	virtual void dotFast(int x, int y, Color color);
	virtual void dot(int x, int y, Color color);
	virtual void dotAdd(int x, int y, Color color);
	virtual void dotMix(int x, int y, Color color);
	virtual Color get(int x, int y);
	virtual void clear(Color color = 0);
	virtual Color** allocateFrameBuffer();
	virtual bool map(int x, int y, int &channel, int &led) = 0;
};
