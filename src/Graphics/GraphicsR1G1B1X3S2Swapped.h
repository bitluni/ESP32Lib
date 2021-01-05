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

class GraphicsR1G1B1X3S2Swapped: public Graphics<ColorR1G1B1A1X4, BLpx1sz8sw2sh0, CTBIdentity>
{
	public:
	//TODO:this must disappear and be tackled in the VGA class
	BufferGraphicsUnit SBits;
	
	GraphicsR1G1B1X3S2Swapped()
	{
		//TODO:decide where to move these.
		SBits = 0xc0;
		frontColor = 0xf;
		defaultBufferValue = SBits;
	}
};
