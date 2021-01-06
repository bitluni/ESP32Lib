/*
	Author: bitluni 2019 Modified by Martin-Laclaustra 2021
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

#include "PinConfig.h"

class VGAPinConfig
{
  public:
	VGAPinConfig(){};

	static const PinConfig VGAv01;
	static const PinConfig VGABlackEdition;
	static const PinConfig VGAWhiteEdition;
	static const PinConfig PicoVGA;
};

