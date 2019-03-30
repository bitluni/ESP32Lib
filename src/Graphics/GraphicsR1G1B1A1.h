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

class GraphicsR1G1B1A1: public Graphics<unsigned char>
{
	public:
	typedef unsigned char Color;
	GraphicsR1G1B1A1()
	{
		frontColor = 0xf;
	}

	virtual int R(Color c) const
	{
		return (c & 1) * 255;
	}
	virtual int G(Color c) const
	{
		return (c & 2) ? 255 : 0;
	}
	virtual int B(Color c) const
	{
		return (c & 4) ? 255 : 0;
	}
	virtual int A(Color c) const
	{
		return (c & 8) ? 255 : 0;
	}

	virtual Color RGBA(int r, int g, int b, int a = 255) const
	{
		return ((r >> 7) & 1) | ((g >> 6) & 2) | ((b >> 5) & 4) | ((a >> 4) & 8);
	}

	virtual void dotFast(int x, int y, Color color)
	{
		if(x & 1)
			backBuffer[y][x >> 1] =  (backBuffer[y][x >> 1] & 0xf) | (color << 4);
		else
			backBuffer[y][x >> 1] = (backBuffer[y][x >> 1] & 0xf0) | (color & 0xf);
	}

	virtual void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
		{
			if(x & 1)
				backBuffer[y][x >> 1] = (backBuffer[y][x >> 1] & 0xf) | (color << 4);
			else
				backBuffer[y][x >> 1] = (backBuffer[y][x >> 1] & 0xf0) | (color & 0xf);
		}
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
		{
			if(x & 1)
				backBuffer[y][x >> 1] = backBuffer[y][x >> 1] | (color << 4);
			else
				backBuffer[y][x >> 1] = backBuffer[y][x >> 1] | (color & 0xf);
		}
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres && (color & 8) != 0)
		{
			if(x & 1)
				backBuffer[y][x >> 1] = (backBuffer[y][x >> 1] & 0xf) | (color << 4);
			else
				backBuffer[y][x >> 1] =  (backBuffer[y][x >> 1] & 0xf0) | (color & 0xf);
		}	
	}
	
	virtual Color get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
		{
			if(x & 1)
				return backBuffer[y][x >> 1] = backBuffer[y][x >> 1] >> 4;
			else
				return backBuffer[y][x >> 1] = backBuffer[y][x >> 1] & 0xf;
		}
		return 0;
	}

	virtual void clear(Color color = 0)
	{
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres / 2; x++)
				this->backBuffer[y][x] = color | (color << 4);
	}

	virtual Color** allocateFrameBuffer()
	{
		return Graphics<Color>::allocateFrameBuffer(xres / 2, yres, (Color)0);
	}
};