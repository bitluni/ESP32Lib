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

class GraphicsR1G1B1A1X2S2Swapped: public Graphics<unsigned char>
{
	public:
	typedef unsigned char Color;
	static const Color RGBAXMask = 0x3f;
	Color SBits;
	
	GraphicsR1G1B1A1X2S2Swapped()
	{
		SBits = 0xc0;
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
		backBuffer[y][x^2] = (color & RGBAXMask) | SBits;
	}

	virtual void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y][x^2] = (color & RGBAXMask) | SBits;
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y][x^2] = backBuffer[y][x^2] | (color & RGBAXMask);
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres && (color & 8) != 0)
			backBuffer[y][x^2] = (color & RGBAXMask) | SBits;
	}
	
	virtual Color get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			return backBuffer[y][x^2] & RGBAXMask;
		return 0;
	}

	virtual void clear(Color color = 0)
	{
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				backBuffer[y][x^2] = (color & RGBAXMask) | SBits;
	}

	virtual Color** allocateFrameBuffer()
	{
		return Graphics<Color>::allocateFrameBuffer(xres, yres, (Color)SBits);
	}
};