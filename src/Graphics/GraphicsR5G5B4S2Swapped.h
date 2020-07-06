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
#include "BufferLayouts/BLpx1sz16sw1sh0.h"
#include "ColorToBuffer/CTBIdentity.h"

class GraphicsR5G5B4S2Swapped: public Graphics<ColorR5G5B4A2, unsigned short>, public BLpx1sz16sw1sh0, public CTBIdentity
{
	public:
	//TODO:this must be abstracted to inherited class after moving most generic code into Graphics class
	typedef typename BLpx1sz16sw1sh0::BufferUnit InternalColor;
	//TODO:this must disappear and be tackled in the VGA class
	InternalColor SBits;

	GraphicsR5G5B4S2Swapped()
	{
		//TODO:decide where to move these.
		SBits = 0xc000;
		frontColor = 0xffff;
	}

	//TODO:eventually (when it is equal for all subclasses) move into a non-virtual function in Graphics class wrapped in a virtual one
	virtual void dotFast(int x, int y, Color color)
	{
		//decide x position[sw] -> shift depending (or not) on x[shval] -> mask[bufferdatamask] -> erase bits
		backBuffer[y][static_sw(x)] &= static_shval(~static_bufferdatamask(), x, y); // delete bits
		//mask[colormask] -> convert to buffer[coltobuf] -> shift depending (or not) on x[shval] -> decide x position[sw] -> store data
		backBuffer[y][static_sw(x)] |= static_shval(coltobuf(color & static_colormask(), x, y), x, y); // write new bits
	}

	//TODO:eventually (when it is equal for all subclasses) move into a non-virtual function in Graphics class wrapped in a virtual one
	virtual Color getFast(int x, int y)
	{
		//decide x position[sw] -> retrieve data -> shift depending (or not) on x[shbuf] -> mask[bufferdatamask] -> convert to color[buftocol]
		return buftocol(static_shbuf(backBuffer[y][static_sw(x)], x, y) & static_bufferdatamask());
	}

	//TODO:study differences between subclasses and decide where it is optimal to allocate buffer
	virtual InternalColor** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer(xres, yres, (InternalColor)SBits);
	}
};
