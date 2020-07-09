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
#include "BufferLayouts/BLpx8sz8swyshy.h"
#include "ColorToBuffer/CTBIdentity.h"

class GraphicsW1: public Graphics<ColorW1X7, unsigned char>, public BLpx8sz8swyshy, public CTBIdentity
{
	public:
	//TODO:this must be abstracted to inherited class after moving most generic code into Graphics class
	typedef typename BLpx8sz8swyshy::BufferUnit InternalColor;
	// FUTURE PLANS: OUTPUTCOLOR COULD BE TEMPLATED
	//These are interpreted as 3-bit color:
	ColorR1G1B1A1X4::Color frontGlobalColor, backGlobalColor;

	GraphicsW1()
	{
		//TODO:decide where to move this.
		frontColor = 0xf;
		frontGlobalColor = 0xf;
		backGlobalColor = 0x0;
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
		return Graphics::allocateFrameBuffer(4*((xres + 3) / 4), (yres + static_ypixperunit() - 1) / static_ypixperunit(), (InternalColor)0);
	}

	virtual void clear(Color color = 0)
	{
		InternalColor storeWord = (color & 0x1) * 0b11111111; // masked for robustness
		for (int y = 0; y < (yres + static_ypixperunit() - 1) / static_ypixperunit(); y++)
			for (int x = 0; x < xres; x++)
				backBuffer[y][x] = storeWord;
	}

	void setFrontGlobalColor(int r, int g, int b, int a = 255)
	{
		frontGlobalColor = ColorR1G1B1A1X4::static_RGBA(r, g, b, a);
	}

	void setBackGlobalColor(int r, int g, int b, int a = 255)
	{
		backGlobalColor = ColorR1G1B1A1X4::static_RGBA(r, g, b, a);
	}
};
