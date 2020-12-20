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

class GraphicsR5G5B4A2: public Graphics<ColorR5G5B4A2, BLpx1sz16sw0sh0, CTBIdentity>
{
	public:

	GraphicsR5G5B4A2()
	{
		//TODO:decide where to move this.
		frontColor = 0xffff;
	}
};
