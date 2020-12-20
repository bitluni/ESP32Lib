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

class GraphicsCA8Swapped: public Graphics<ColorR8G8B8A8, BLpx1sz8sw2sh0, CTBComposite>
{
	public:

	GraphicsCA8Swapped()
	{
		//TODO:decide where to move these.
		frontColor = 0xffffffff;
		defaultBufferValue = levelBlack;
	}

	virtual void dotAdd(int x, int y, Color color)
	{
		dot(x, y, color);
	}

	virtual void dotMix(int x, int y, Color color)
	{
		dot(x, y, color);
	}
};
