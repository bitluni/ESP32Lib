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
#include "BufferLayouts/BLpx1sz8sw0sh0.h"
#include "ColorToBuffer/CTBIdentity.h"

class GraphicsTextBuffer: public Graphics<ColorW8, unsigned char>, public BLpx1sz8sw0sh0, public CTBIdentity
{
	public:
	//TODO:this must be abstracted to inherited class after moving most generic code into Graphics class
	typedef typename BLpx1sz8sw0sh0::BufferUnit BufferUnit;
	//These are interpreted as 3-bit color:
	ColorR1G1B1A1X4::Color frontGlobalColor, backGlobalColor;

	GraphicsTextBuffer()
	{
		//TODO:decide where to move this.
		frontColor = 0x30;
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
	virtual BufferUnit** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer(xres, yres, (BufferUnit)32);
	}

	virtual void clear(Color color = 32)
	{
		Graphics::clear(color);
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		dot(x, y, color);
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		dot(x, y, color);
	}

	void setFrontGlobalColor(int r, int g, int b, int a = 255)
	{
		frontGlobalColor = ColorR1G1B1A1X4::static_RGBA(r, g, b, a);
	}

	void setBackGlobalColor(int r, int g, int b, int a = 255)
	{
		backGlobalColor = ColorR1G1B1A1X4::static_RGBA(r, g, b, a);
	}

	void setTextColor(ColorR1G1B1A1X4::Color front, ColorR1G1B1A1X4::Color back = 0)
	{
		frontGlobalColor = front;
		backGlobalColor = back;
	}

	virtual void setFont(Font &font)
	{
		if (this->font != 0) return; // font can not be changed in this mode
		this->font = &font;
		cursorXIncrement = 1;
		cursorYIncrement = 1;
	}

	virtual void drawChar(int x, int y, int ch)
	{
		if (!font)
			return;
		if (!font->valid(ch))
			return;
		dot(x, y, ch);
	}
};
