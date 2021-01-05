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

class GraphicsW8RangedSwapped: public Graphics<ColorW8, BLpx1sz8sw2sh0, CTBRange>
{
	public:

	GraphicsW8RangedSwapped()
	{
		//TODO:decide where to move this.
		frontColor = 0xff;
		defaultBufferValue = colorMinValue;
	}

	virtual void clear(Color color = 0)
	{
		BufferGraphicsUnit newColor = (BufferGraphicsUnit)static_shval(coltobuf(color & static_colormask(), 0, 0), 0, 0);
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				backBuffer[y][x] = newColor;
	}
};
