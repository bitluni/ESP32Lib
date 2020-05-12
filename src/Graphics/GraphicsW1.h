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
#include "Graphics.h"

class GraphicsW1: public Graphics<ColorW1X7, unsigned char>
{
	public:
	typedef unsigned char InternalColor;
	// FUTURE PLANS: OUTPUTCOLOR COULD BE TEMPLATED
	//These are interpreted as 3-bit color:
	ColorR1G1B1A1X4::Color frontGlobalColor, backGlobalColor;

	GraphicsW1()
	{
		frontColor = 0xf;
		storageCoefficient = 8;
		frontGlobalColor = 0xf;
		backGlobalColor = 0x0;
	}

	virtual void dotFast(int x, int y, Color color)
	{
		InternalColor bitmask = 0x80 >> (y & 0x7);
		backBuffer[y >> 3][x] = (backBuffer[y >> 3][x] & (0xff ^ bitmask)) | (bitmask * (color & 0x1)); // masked for robustness
	}

	virtual Color getFast(int x, int y)
	{
		return (backBuffer[y >> 3][x] >> (0x7 - (y & 0x7))) & 0x1;
	}

	virtual InternalColor** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer(4*((xres + 3) / 4), (yres + storageCoefficient - 1) / storageCoefficient, (InternalColor)0);
	}

	virtual void clear(Color color = 0)
	{
		InternalColor storeWord = (color & 0x1) * 0b11111111; // masked for robustness
		for (int y = 0; y < (yres + storageCoefficient - 1) / storageCoefficient; y++)
			for (int x = 0; x < xres; x++)
				backBuffer[y][x] = storeWord;
	}

	void setFrontGlobalColor(int r, int g, int b, int a = 255)
	{
		frontGlobalColor = ColorR1G1B1A1X4::static_RGBA(r, g, b, a);
	}

	void setBackGlobalColor(int r, int g, int b, int a = 255)
	{
		backGlobalColor = ColorR1G1B1A1X4::static_RGBA(r, g, b, a);
	}
};
