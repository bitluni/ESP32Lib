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

class GraphicsX6S2W8Swapped: public Graphics<unsigned short>
{
	public:
	typedef unsigned short Color;
	static const Color RGBAXMask = 0xff3f;
	Color SBits;
	int colorDepthConversionFactor = 256;
	int colorMinValue = 0;
	int colorMaxValue = 255;


	GraphicsX6S2W8Swapped()
	{
		SBits = 0xc0;
		frontColor = 0xff00;
	}

	virtual int R(Color c) const
	{
		return c >> 8;
	}
	virtual int G(Color c) const
	{
		return c >> 8;
	}
	virtual int B(Color c) const
	{
		return c >> 8;
	}
	virtual int A(Color c) const
	{
		return 255;
	}

	virtual Color RGBA(int r, int g, int b, int a = 255) const
	{
		return (r * 2126 + g * 7152 + b * 722) / 10000;
	}

	virtual void dotFast(int x, int y, Color color)
	{
		backBuffer[y][x^1] = ((((colorMinValue<<8) + colorDepthConversionFactor*(int)color) & 0xFF00) & RGBAXMask) | SBits;
	}

	virtual void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			dotFast(x, y, color);
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
		{
			int newColor = get(x, y);
			newColor += color;
			dotFast(x, y, (newColor > 0xff) ? 0xff : newColor);
		}
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres && (color > 0))
			dotFast(x, y, ((int)get(x, y) + color)>>1);
	}
	
	virtual Color get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			return (((backBuffer[y][x^1] & RGBAXMask) & 0xFF00) - (colorMinValue<<8)) / colorDepthConversionFactor;
		return 0;
	}

	virtual void clear(Color color = 0)
	{
		Color newColor = ((((colorMinValue<<8) + colorDepthConversionFactor*(int)color) & 0xFF00) & RGBAXMask) | SBits;
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				this->backBuffer[y][x] = newColor;
	}

	virtual Color** allocateFrameBuffer()
	{
		return Graphics<Color>::allocateFrameBuffer(xres, yres, (Color)SBits);
	}
};
