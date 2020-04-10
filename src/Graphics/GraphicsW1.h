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

class GraphicsW1: public Graphics<unsigned char>
{
	public:
	typedef unsigned char Color;
	//These are interpreted as 3-bit color:
	Color frontGlobalColor, backGlobalColor;

	void setFrontGlobalColor(int r, int g, int b, int a = 255)
	{
		frontGlobalColor = ((r >> 7) & 1) | ((g >> 6) & 2) | ((b >> 5) & 4) | ((a >> 4) & 8);
	}

	void setBackGlobalColor(int r, int g, int b, int a = 255)
	{
		backGlobalColor = ((r >> 7) & 1) | ((g >> 6) & 2) | ((b >> 5) & 4) | ((a >> 4) & 8);
	}

	GraphicsW1()
	{
		frontColor = 0xf;
		frontGlobalColor = 0xf;
		backGlobalColor = 0;
	}

	virtual int R(Color c) const
	{
		return (c & 1) ? 255 : 0;
	}
	virtual int G(Color c) const
	{
		return (c & 1) ? 255 : 0;
	}
	virtual int B(Color c) const
	{
		return (c & 1) ? 255 : 0;
	}
	virtual int A(Color c) const
	{
		return (c & 1) ? 255 : 0;
	}

	virtual Color RGBA(int r, int g, int b, int a = 255) const
	{
		return ((r > 0) | (g > 0) | (b > 0)) & (a > 0);
	}

	virtual void dotFast(int x, int y, Color color)
	{
		backBuffer[y >> 3][x] = (backBuffer[y >> 3][x] & (0xff ^ (0x80 >> (y & 0x7)))) | ((0x80 >> (y & 0x7))*(color & 1));
	}

	virtual void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y >> 3][x] = (backBuffer[y >> 3][x] & (0xff ^ (0x80 >> (y & 0x7)))) | ((0x80 >> (y & 0x7))*(color & 1));
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y >> 3][x] = backBuffer[y >> 3][x] | ((0x80 >> (y & 0x7))*(color & 1));
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres && (color & 1) != 0)
			backBuffer[y >> 3][x] = (backBuffer[y >> 3][x] & (0xff ^ (0x80 >> (y & 0x7)))) | ((0x80 >> (y & 0x7))*(color & 1));
	}
	
	virtual Color get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			return (backBuffer[y >> 3][x] >> (0x7 - (y & 0x7))) & 0x1;
		return 0;
	}

	virtual void clear(Color color = 0)
	{
		for (int y = 0; y < this->yres / 8; y++)
			for (int x = 0; x < this->xres; x++)
				this->backBuffer[y][x] = (color & 1) * 255;
	}

	virtual Color** allocateFrameBuffer()
	{
		return Graphics<Color>::allocateFrameBuffer(4*((xres + 3) / 4), (yres + 7) / 8, (Color)0);
	}
};
