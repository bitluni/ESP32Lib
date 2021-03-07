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

	static const ModeComposite MODEPAL288P; // OOM in CompositeGrayPDM8/4-8266
	static const ModeComposite MODEPALHalf144P; // insufficient sync in CompositeGrayPDM2-8266
	static const ModeComposite MODEPALQuarter144P; // CompositeGrayPDM8-8266 (stripes), CompositeGrayPDM4/2-8266 (unusable stripes)
	static const ModeComposite MODEPAL288Pmax; // OOM in DAC, in 8266
	static const ModeComposite MODEPAL288Pmin; // does not work in ladder, CompositeGrayPDM8/4-8266 (unusable stripes), insufficient sync in CompositeGrayPDM2-8266
	static const ModeComposite MODEPAL576I; // OOM in DAC, in 8266
	static const ModeComposite MODEPAL576Imax; // OOM in DAC, in Ladder, in 8266
	static const ModeComposite MODEPAL576Imin; // does not work in ladder, OOM in CompositeGrayPDM8-8266, CompositeGrayPDM4-8266 (unusable stripes), insufficient sync in CompositeGrayPDM2-8266
	static const ModeComposite MODEPAL576Idiv3; // OOM in CompositeGrayPDM8/4-8266
	static const ModeComposite MODEPALColor288P; // OOM in CompositeGrayPDM8/4-8266
	static const ModeComposite MODEPALColor288Pmid; // OOM in CompositeGrayPDM8/4-8266

	static const ModeComposite MODENTSC240P; // OOM in CompositeGrayPDM8/4-8266, incorrect sync in CompositeGrayPDM2-8266
	static const ModeComposite MODENTSCHalf120P; // incorrect sync in CompositeGrayPDM?-8266
	static const ModeComposite MODENTSC240Pmax;
	static const ModeComposite MODENTSC480I; // OOM in DAC, in 8266
	static const ModeComposite MODENTSC480Imax; // OOM in DAC, in Ladder, in 8266
	static const ModeComposite MODENTSCColor240P;
	static const ModeComposite MODENTSCColor120P;

	//OLD MODES

	static const ModeComposite MODE400x300;
	static const ModeComposite MODEPAL312P;
};

