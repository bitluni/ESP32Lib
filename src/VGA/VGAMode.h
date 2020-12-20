/*
	Author: bitluni 2019 Modified by Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

#include "Mode.h"

class VGAMode
{
  public:
	VGAMode(){};

	static const Mode MODE320x480;
	static const Mode MODE320x240;
	static const Mode MODE320x120;
	static const Mode MODE320x400;
	static const Mode MODE320x200;
	static const Mode MODE360x400;
	static const Mode MODE360x200;
	static const Mode MODE360x350;
	static const Mode MODE360x175;

	static const Mode MODE320x350;
	static const Mode MODE320x175;

	static const Mode MODE400x300;
	static const Mode MODE400x150;
	static const Mode MODE400x100;
	static const Mode MODE200x150;

	static const Mode MODE500x480;
	static const Mode MODE500x240;

	static const Mode MODE1280x1024;
	static const Mode MODE1280x960;
	static const Mode MODE1280x800;
	static const Mode MODE1024x768;
	static const Mode MODE800x600;
	static const Mode MODE720x400;
	static const Mode MODE720x350;
	static const Mode MODE640x480;
	static const Mode MODE640x400;
	static const Mode MODE640x350;
};

