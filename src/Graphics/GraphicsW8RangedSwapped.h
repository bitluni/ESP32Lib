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

class GraphicsW8RangedSwapped: public Graphics<unsigned char>
{
	public:
	typedef unsigned char Color;
	int colorDepthConversionFactor = 256;
	int colorMinValue = 0;
	int colorMaxValue = 255;

	GraphicsW8RangedSwapped()
	{
		frontColor = 0xff;
	}

	virtual int R(Color c) const
	{
		return c;
	}
	virtual int G(Color c) const
	{
		return c;
	}
	virtual int B(Color c) const
	{
		return c;
	}
	virtual int A(Color c) const
	{
		return 255;
	}

	virtual Color RGBA(int r, int g, int b, int a = 255) const
	{
		return (r * 2126 + g * 7152 + b * 722) * colorDepthConversionFactor / 256 / 10000;
	}

	virtual void dotFast(int x, int y, Color color)
	{
		backBuffer[y][x^2] = ((colorMinValue<<8) + colorDepthConversionFactor*(int)color)>>8;
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
			return (Color)((((int)backBuffer[y][x^2] << 8) - (colorMinValue<<8)) / colorDepthConversionFactor);
		return 0;
	}

	virtual void clear(Color color = 0)
	{
		Color newColor = ((colorMinValue<<8) + colorDepthConversionFactor*(int)color)>>8;
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				backBuffer[y][x^2] = newColor;
	}

	virtual Color** allocateFrameBuffer()
	{
		return Graphics<Color>::allocateFrameBuffer(xres, yres, (Color)colorMinValue);
	}
};
