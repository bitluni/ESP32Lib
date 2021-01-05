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

class GraphicsW1: public Graphics<ColorW1X7, BLpx8sz8swyshy, CTBIdentity>
{
	public:
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

	virtual void clear(Color color = 0)
	{
		BufferGraphicsUnit storeWord = (color & 0x1) * 0b11111111; // masked for robustness
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
