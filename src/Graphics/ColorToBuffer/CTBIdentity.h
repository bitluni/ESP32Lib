/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

class CTBIdentity
{
	public:
	CTBIdentity() {}

	static int coltobuf(int val, int /*x*/, int /*y*/)
	{
		return val;
	}
	static int buftocol(int val)
	{
		return val;
	}
};
