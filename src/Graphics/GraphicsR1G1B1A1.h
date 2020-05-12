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

class GraphicsR1G1B1A1: public Graphics<ColorR1G1B1A1X4, unsigned char>
{
	public:
	typedef unsigned char InternalColor;

	GraphicsR1G1B1A1()
	{
		frontColor = 0xf;
		storageCoefficient = 2;
	}

	virtual void dotFast(int x, int y, Color color)
	{
		if(x & 1)
			backBuffer[y][x >> 1] =  (backBuffer[y][x >> 1] & 0xf) | (color << 4);
		else
			backBuffer[y][x >> 1] = (backBuffer[y][x >> 1] & 0xf0) | (color & 0xf); // masked high-nibble for robustness
	}

	virtual Color getFast(int x, int y)
	{
		if(x & 1)
			return backBuffer[y][x >> 1] >> 4;
		else
			return backBuffer[y][x >> 1] & 0xf;
	}

	virtual InternalColor** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer((xres + storageCoefficient - 1) / storageCoefficient, yres, (InternalColor)0);
	}

	virtual void clear(Color color = 0)
	{
		unsigned char storeWord = (color & 0xf) * 0b00010001; // masked high-nibble for robustness
		for (int y = 0; y < yres; y++)
			for (int x = 0; x < (xres + storageCoefficient - 1) / storageCoefficient; x++)
				backBuffer[y][x] = storeWord;
	}
};
