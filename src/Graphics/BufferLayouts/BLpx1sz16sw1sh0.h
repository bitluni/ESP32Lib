/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

// px pixels per unit
// sz unit size in bits
// sw nor swap or shift of x or y
// sh shift bits within unit
class BLpx1sz16sw1sh0
{
	public:
	typedef unsigned short BufferUnit;
	BLpx1sz16sw1sh0() {}

	static const int static_xpixperunit()
	{
		return 1;
	}

	static const int static_ypixperunit()
	{
		return 1;
	}

	static const int static_bufferdatamask()
	{
		return 0b1111111111111111;
	}

	static const int static_replicate()
	{
		return 1;
	}
	static const int static_replicate32()
	{
		return 0x00010001;
	}

	static int static_swx(int x)
	{
		return x^1;
	}
	static int static_swy(int y)
	{
		return y;
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
