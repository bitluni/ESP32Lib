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

class GraphicsR1G1B1A1: public Graphics<ColorR1G1B1A1X4, BLpx2sz8swxshx, CTBIdentity>
{
	public:

	GraphicsR1G1B1A1()
	{
		//TODO:decide where to move this.
		frontColor = 0xf;
	}

	//TODO:study differences between subclasses and decide where it is optimal to allocate buffer
	virtual BufferUnit** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer((xres + static_xpixperunit() - 1) / static_xpixperunit(), yres, (BufferUnit)0);
	}

	virtual void clear(Color color = 0)
	{
		unsigned char storeWord = (color & 0xf) * 0b00010001; // masked high-nibble for robustness
		for (int y = 0; y < yres; y++)
			for (int x = 0; x < (xres + static_xpixperunit() - 1) / static_xpixperunit(); x++)
				backBuffer[y][x] = storeWord;
	}
};
