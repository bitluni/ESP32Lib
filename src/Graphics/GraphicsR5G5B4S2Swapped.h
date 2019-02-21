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

class GraphicsR5G5B4S2Swapped: public Graphics<unsigned short>
{
	public:
	typedef unsigned short Color;
	static const Color RGBMask = 0x3fff;
	Color SBits;

	GraphicsR5G5B4S2Swapped()
	{
		SBits = 0xc000;
		frontColor = 0xffff;
	}

	virtual int R(Color c) const
	{
		return (((c << 1) & 0x3e) * 255 + 1) / 0x3e;
	}
	virtual int G(Color c) const
	{
		return (((c >> 4) & 0x3e) * 255 + 1) / 0x3e;
	}
	virtual int B(Color c) const
	{
		return (((c >> 9) & 0x1e) * 255 + 1) / 0x1e;
	}
	virtual int A(Color c) const
	{
		return (((c >> 13) & 6) * 255 + 1) / 6;
	}

	virtual Color RGBA(int r, int g, int b, int a = 255) const
	{
		return ((r >> 3) & 0b11111) | ((g << 2) & 0b1111100000) | ((b << 6) & 0b11110000000000) | ((a << 8) & 0xc000);
	}

	virtual void dotFast(int x, int y, Color color)
	{
		backBuffer[y][x^1] = (color & RGBMask) | SBits;
	}

	virtual void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y][x^1] = (color & RGBMask) | SBits;
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
		{
			int c0 = backBuffer[y][x^1];
			int c1 = color;
			int r = (c0 & 0b11111) + (c1 & 0b11111);
			if(r > 0b11111) r = 0b11111;
			int g = (c0 & 0b1111100000) + (c1 & 0b1111100000);
			if(g > 0b1111100000) g = 0b1111100000;
			int b = (c0 & 0b11110000000000) + (c1 & 0b11110000000000);
			if(b > 0b11110000000000) b = 0b11110000000000;
			backBuffer[y][x^1] = r | (g & 0b1111100000) | (b & 0b11110000000000) | SBits;
		}
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres && (color >> 14) != 0)
		{
			unsigned int ai = (3 - (color >> 14)) * (65536 / 3);
			unsigned int a = 65536 - ai;
			unsigned int co = backBuffer[y][x^1];
			unsigned int ro = (co & 0b11111) * ai;
			unsigned int go = (co & 0b1111100000) * ai;
			unsigned int bo = (co & 0b11110000000000) * ai;
			unsigned int r = (color & 0b11111) * a + ro;
			unsigned int g = ((color & 0b1111100000) * a + go) & 0b11111000000000000000000000;
			unsigned int b = ((color & 0b11110000000000) * a + bo) & 0b111100000000000000000000000000;
			backBuffer[y][x^1] = ((r | g | b) >> 16) | SBits;
		}	
	}
	
	virtual Color get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			return backBuffer[y][x^1] & RGBMask;
		return 0;
	}

	virtual void clear(Color color = 0)
	{
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				backBuffer[y][x^1] = (color & RGBMask) | SBits;
	}

	virtual Color** allocateFrameBuffer()
	{
		return Graphics<Color>::allocateFrameBuffer(xres, yres, (Color)SBits);
	}
};