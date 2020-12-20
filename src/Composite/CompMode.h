/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#pragma once

#include "ModeComposite.h"

class CompMode
{
  public:
	CompMode(){};

	static const ModeComposite MODEPAL288P;
	static const ModeComposite MODEPAL576I;
	static const ModeComposite MODEPALColor288P;

	static const ModeComposite MODENTSC240P;
	static const ModeComposite MODENTSC480I;
	static const ModeComposite MODENTSCColor240P;

	static const ModeComposite MODEPAL288Pmax;

	static const ModeComposite MODEPAL288Pmin;
	static const ModeComposite MODEPAL576Imin;

	static const ModeComposite MODEPAL576Idiv3;

	static const ModeComposite MODENTSC240Pmax;
};

