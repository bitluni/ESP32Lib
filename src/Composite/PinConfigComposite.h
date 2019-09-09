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

class PinConfigComposite
{
  public:
	int c0, c1, c2, c3, c4, c5, c6, c7;
	PinConfigComposite(
		const int c0 = -1,
		const int c1 = -1,
		const int c2 = -1,
		const int c3 = -1,
		const int c4 = -1,
		const int c5 = -1,
		const int c6 = -1,
		const int c7 = -1)
		: c0(c0),
		  c1(c1),
		  c2(c2),
		  c3(c3),
		  c4(c4),
		  c5(c5),
		  c6(c6),
		  c7(c7)
	{
	}
	void fill(int *pins) const
	{
		pins[0] = c0;
		pins[1] = c1;
		pins[2] = c2;
		pins[3] = c3;
		pins[4] = c4;
		pins[5] = c5;
		pins[6] = c6;
		pins[7] = c7;
	}
};