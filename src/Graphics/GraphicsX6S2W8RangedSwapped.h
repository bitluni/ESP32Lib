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
#include "BufferLayouts/BLpx1sz16sw1sh8.h"
#include "ColorToBuffer/CTBRange.h"

class GraphicsX6S2W8RangedSwapped: public Graphics<ColorW8, unsigned short>, public BLpx1sz16sw1sh8, public CTBRange
{
	public:
	//TODO:this must be abstracted to inherited class after moving most generic code into Graphics class
	typedef typename BLpx1sz16sw1sh8::BufferUnit InternalColor;
	//TODO:this must disappear and be tackled in the VGA class
	InternalColor SBits;

	GraphicsX6S2W8RangedSwapped()
	{
		//TODO:decide where to move these.
		SBits = 0x00c0;
		frontColor = 0xff;
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
		return Graphics::allocateFrameBuffer(xres, yres, (InternalColor)(colorMinValue<<8)|SBits);
	}

	virtual void clear(Color color = 0)
	{
		InternalColor newColor = (InternalColor)static_shval(coltobuf(color & static_colormask(), 0, 0), 0, 0) | SBits;
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				backBuffer[y][x] = newColor;
	}
};
