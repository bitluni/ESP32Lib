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

class GraphicsTextBuffer: public Graphics<ColorW8, BLpx1sz8sw0sh0, CTBIdentity>
{
	public:
	//These are interpreted as 3-bit color:
	ColorR1G1B1A1X4::Color frontGlobalColor, backGlobalColor;

	GraphicsTextBuffer()
	{
		//TODO:decide where to move this.
		frontColor = 0x30;
		frontGlobalColor = 0xf;
		backGlobalColor = 0x0;
		defaultBufferValue = 32;
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
