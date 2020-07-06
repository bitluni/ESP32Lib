/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

// BLpx1sz16sw1sh0
// pixels per unit 1
// size 16 bits
// swap bit1 (^2) of x
// shift unshifted
class BLpx1sz8sw2sh0
{
	public:
	typedef unsigned char BufferUnit;
	BLpx1sz8sw2sh0() {}

	static const int static_bufferdatamask()
	{
		return 0b00111111;
	}

	static const int static_replicate()
	{
		return 1;
	}
	static const int static_replicate32()
	{
		return 0x01010101;
	}

	static int static_sw(int x)
	{
		return x^2;
	}
	static int static_shval(BufferUnit val, int /*x*/, int /*y*/)
	{
		return val;
	}
	static int static_shbuf(BufferUnit val, int /*x*/, int /*y*/)
	{
		return val;
	}
};
