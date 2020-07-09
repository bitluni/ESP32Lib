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
#include "BufferLayouts/BLpx2sz8swxshx.h"
#include "ColorToBuffer/CTBIdentity.h"

class GraphicsR1G1B1A1: public Graphics<ColorR1G1B1A1X4, unsigned char>, public BLpx2sz8swxshx, public CTBIdentity
{
	public:
	//TODO:this must be abstracted to inherited class after moving most generic code into Graphics class
	typedef typename BLpx2sz8swxshx::BufferUnit InternalColor;

	GraphicsR1G1B1A1()
	{
		//TODO:decide where to move this.
		frontColor = 0xf;
	}

	//TODO:eventually (when it is equal for all subclasses) move into a non-virtual function in Graphics class wrapped in a virtual one
	virtual void dotFast(int x, int y, Color color)
	{
		//decide x position[sw] -> shift depending (or not) on x[shval] -> mask[bufferdatamask] -> erase bits
		backBuffer[static_swy(y)][static_swx(x)] &= ~static_shval(static_colormask(), x, y); // delete bits
		//mask[colormask] -> convert to buffer[coltobuf] -> shift depending (or not) on x[shval] -> decide x position[sw] -> store data
		backBuffer[static_swy(y)][static_swx(x)] |= static_shval(coltobuf(color & static_colormask(), x, y), x, y); // write new bits
	}

	//TODO:eventually (when it is equal for all subclasses) move into a non-virtual function in Graphics class wrapped in a virtual one
	virtual Color getFast(int x, int y)
	{
		//decide x position[sw] -> retrieve data -> shift depending (or not) on x[shbuf] -> mask[bufferdatamask] -> convert to color[buftocol]
		return buftocol(static_shbuf(backBuffer[static_swy(y)][static_swx(x)], x, y) & static_colormask());
	}

	//TODO:study differences between subclasses and decide where it is optimal to allocate buffer
	virtual InternalColor** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer((xres + static_xpixperunit() - 1) / static_xpixperunit(), yres, (InternalColor)0);
	}

	virtual void clear(Color color = 0)
	{
		unsigned char storeWord = (color & 0xf) * 0b00010001; // masked high-nibble for robustness
		for (int y = 0; y < yres; y++)
			for (int x = 0; x < (xres + static_xpixperunit() - 1) / static_xpixperunit(); x++)
				backBuffer[y][x] = storeWord;
	}
};
