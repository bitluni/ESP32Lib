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

class GraphicsR5G5B4S2Swapped: public Graphics<ColorR5G5B4A2, BLpx1sz16sw1sh0, CTBIdentity>
{
	public:
	BufferUnit SBits;

	GraphicsR5G5B4S2Swapped()
	{
		//TODO:decide where to move these.
		SBits = 0xc000;
		frontColor = 0xffff;
	}

	//TODO:study differences between subclasses and decide where it is optimal to allocate buffer
	virtual BufferUnit** allocateFrameBuffer()
	{
		return Graphics::allocateFrameBuffer(xres, yres, (BufferUnit)SBits);
	}
};
