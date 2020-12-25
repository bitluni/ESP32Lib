/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once
#include "Graphics.h"

class GraphicsM8CA8Swapped: public Graphics<ColorR8G8B8A8, BLpx1sz16sw1sh0, CTBCompositeMemory>
{
	public:

	GraphicsM8CA8Swapped()
	{
		//TODO:decide where to move these.
		frontColor = 0xffffffff;
		defaultBufferValue = (int)levelBlack<<8;
	}
};
