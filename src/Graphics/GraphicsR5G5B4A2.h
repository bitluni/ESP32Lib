/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 2.0
	https://creativecommons.org/licenses/by-sa/2.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once
#include "Graphics.h"
#include "TriangleTree.h"

class GraphicsR5G5B4A2: public Graphics<unsigned short>
{
	public:
	typedef unsigned short Color;
	
	GraphicsR5G5B4A2()
	{
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
		return ((r >> 3) & 0b11111) | ((g << 2) & 0b1111100000) | ((b << 6) & 0b11110000000000) | ((a << 8) & 0b110000000000000000);
	}

	virtual void dotFast(int x, int y, Color color)
	{
		backBuffer[y][x] = color;
	}

	virtual void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y][x] = color;
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		//todo repair this
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y][x] = color + backBuffer[y][x];
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres && (color >> 14) != 0)
		{
			unsigned int ai = (3 - (color >> 14)) * (65536 / 3);
			unsigned int a = 65536 - ai;
			unsigned int co = backBuffer[y][x];
			unsigned int ro = (co & 0b11111) * ai;
			unsigned int go = (co & 0b1111100000) * ai;
			unsigned int bo = (co & 0b11110000000000) * ai;
			unsigned int r = (color & 0b11111) * a + ro;
			unsigned int g = ((color & 0b1111100000) * a + go) & 0b11111000000000000000000000;
			unsigned int b = ((color & 0b11110000000000) * a + bo) & 0b111100000000000000000000000000;
			backBuffer[y][x] = (r | g | b) >> 16;
		}	
	}
	
	virtual Color get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			return backBuffer[y][x];
		return 0;
	}

	virtual void clear(Color clear = 0)
	{
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				backBuffer[y][x] = clear;
	}

	virtual Color** allocateFrameBuffer()
	{
		Color** frame = (Color **)malloc(yres * sizeof(Color *));
		for (int y = 0; y < yres; y++)
		{
			frame[y] = (Color *)malloc(xres * sizeof(Color));
			for (int x = 0; x < xres; x++)
				frame[y][x] = 0;
		}
		return frame;
	}
};