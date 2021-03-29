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

class GraphicsX6S2W8RangedSwapped: public Graphics<ColorW8, BLpx1sz16sw1sh8, CTBRange>
{
	public:
	//TODO:this must disappear and be tackled in the VGA class
	BufferGraphicsUnit SBits;

	GraphicsX6S2W8RangedSwapped()
	{
		//TODO:decide where to move these.
		SBits = 0x00c0;
		frontColor = 0xff;
		defaultBufferValue = ((int)colorMinValue<<8)|SBits;
	}

	virtual void clear(Color color = 0)
	{
		BufferGraphicsUnit bufferUnaffectedBits = (backBuffer[0][0])&( ~graphics_shval(Graphics::static_bufferdatamask(), 0, 0) );
		BufferGraphicsUnit newColor = (BufferGraphicsUnit)static_shval(coltobuf(color & static_colormask(), 0, 0), 0, 0) | bufferUnaffectedBits;
		for (int y = 0; y < this->yres; y++)
			for (int x = 0; x < this->xres; x++)
				backBuffer[y][x] = newColor;
	}
};
