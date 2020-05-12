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

class GraphicsX6S2W8RangedSwapped: public Graphics<ColorW8, unsigned short>
{
	public:
	typedef unsigned short InternalColor;
	static const InternalColor RGBAXMask = 0xff3f;
	InternalColor SBits;
	int colorDepthConversionFactor = 256;
	int colorMinValue = 0;
	int colorMaxValue = 255;


	GraphicsX6S2W8RangedSwapped()
	{
		SBits = 0x00c0;
		frontColor = 0xff;
	}

	virtual void dotFast(int x, int y, Color color)
	{
		backBuffer[y][x^1] = (InternalColor)((((colorMinValue<<8) + colorDepthConversionFactor*(int)color) & 0xff00) & RGBAXMask) | SBits;
	}

	virtual Color getFast(int x, int y)
	{
		return (Color)((((backBuffer[y][x^1] & RGBAXMask) & 0xff00) - (colorMinValue<<8)) / colorDepthConversionFactor);
	}

	virtual InternalColor** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer(xres, yres, (InternalColor)(colorMinValue<<8)|SBits);
	}

	virtual void clear(Color color = 0)
	{
		InternalColor newColor = (InternalColor)((((colorMinValue<<8) + colorDepthConversionFactor*(int)color) & 0xff00) & RGBAXMask) | SBits;
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				backBuffer[y][x] = newColor;
	}
};
