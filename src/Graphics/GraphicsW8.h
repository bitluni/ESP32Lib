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

class GraphicsW8: public Graphics<ColorW8, BLpx1sz8sw0sh0, CTBIdentity>
{
	public:

	GraphicsW8()
	{
		//TODO:decide where to move this.
		frontColor = 0xff;
	}
};
