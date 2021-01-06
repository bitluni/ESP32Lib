/*
	Author: bitluni 2019 Modified by Martin-Laclaustra 2021
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#include "VGAPinConfig.h"

//PinConfig arguments order:
//==============================
//r0, r1, r2, r3, r4;
//g0, g1, g2, g3, g4;
//b0, b1, b2, b3;
//hSync, vSync, clock;

const PinConfig VGAPinConfig::VGAv01(2, 4, 12, 13, 14,  15, 16, 17, 18, 19,  21, 22, 23, 27,  32, 33,  -1);
const PinConfig VGAPinConfig::VGABlackEdition(2, 4, 12, 13, 14,  15, 16, 17, 18, 19,  21, 22, 23, 27,  32, 33,  -1);
const PinConfig VGAPinConfig::VGAWhiteEdition(5, 14, 13, 15, 2,  19, 18, 17, 4, 16,  27, 22, 12, 21,  32, 33, -1);
const PinConfig VGAPinConfig::PicoVGA(-1, -1, -1, 18, 5,  -1, -1, -1, 14, 4,  -1, -1, 27, 15,  32, 33,  -1);

