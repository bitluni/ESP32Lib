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

class GraphicsTextBuffer: public Graphics<unsigned char>
{
	public:
	typedef unsigned char Color; //Color are ASCII codes in this buffer
	//These are interpreted as 3-bit color:
	Color frontGlobalColor, backGlobalColor;

	void setFrontGlobalColor(int r, int g, int b, int a = 255)
	{
		frontGlobalColor = ((r >> 7) & 1) | ((g >> 6) & 2) | ((b >> 5) & 4) | ((a >> 4) & 8);
	}

	void setBackGlobalColor(int r, int g, int b, int a = 255)
	{
		backGlobalColor = ((r >> 7) & 1) | ((g >> 6) & 2) | ((b >> 5) & 4) | ((a >> 4) & 8);
	}

	void setTextColor(long front, long back = 0)
	{
		frontGlobalColor = frontColor = front;
		backGlobalColor = backColor = back;
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

	GraphicsTextBuffer()
	{
		frontColor = 0xf;
		frontGlobalColor = 0xf;
		backGlobalColor = 0;
	}

	virtual int R(Color c) const
	{
		return (c & 1) ? 255 : 0;
	}
	virtual int G(Color c) const
	{
		return (c & 1) ? 255 : 0;
	}
	virtual int B(Color c) const
	{
		return (c & 1) ? 255 : 0;
	}
	virtual int A(Color c) const
	{
		return (c & 1) ? 255 : 0;
	}

	virtual Color RGBA(int r, int g, int b, int a = 255) const
	{
		return ((r > 0) | (g > 0) | (b > 0)) & (a > 0);
	}

	virtual void dotFast(int x, int y, Color color)
	{
		backBuffer[y][x] = color;
	}

	virtual void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y][x] = color;
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backBuffer[y][x] = color;
	}
	
	virtual void dotMix(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres && (color & 1) != 0)
			backBuffer[y][x] = color;
	}
	
	virtual Color get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			return backBuffer[y][x];
		return 0;
	}

	virtual void clear(Color color = 32)
	{
		Graphics::clear(color);
	}

	virtual Color** allocateFrameBuffer()
	{
		return Graphics<Color>::allocateFrameBuffer(xres, yres, (Color)32);
	}
};
