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

class GraphicsR1G1B1X3S2Swapped: public Graphics<ColorR1G1B1A1X4, unsigned char>
{
	public:
	typedef unsigned char InternalColor;
	static const InternalColor RGBAXMask = 0x3f;
	InternalColor SBits;
	
	GraphicsR1G1B1X3S2Swapped()
	{
		frontColor = 0xf;
		SBits = 0xc0;
	}

	virtual void dotFast(int x, int y, Color color)
	{
		backBuffer[y][x^2] = (color & RGBAXMask) | SBits;
	}

	virtual Color getFast(int x, int y)
	{
		return backBuffer[y][x^2] & RGBAXMask;
	}

	virtual InternalColor** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer(xres, yres, (InternalColor)SBits);
	}
};
