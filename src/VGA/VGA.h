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

#include "Mode.h"
#include "PinConfig.h"

// for back compatibility reasons, allow recalling "pre-configured" modes and PinConfigs within the class
// it might be deprecated in a major version increase
#include "VGAMode.h"
#include "VGAPinConfig.h"

// This class must deal with the technical requirements for the signal,
// with the physical details of the output,
// and with the common interface

// for back compatibility reasons, allow recalling "pre-configured" modes and PinConfigs within the class
// by inheriting VGAMode and VGAPinConfig classes, it might be deprecated in a major version increase
class VGA : public VGAMode, public VGAPinConfig
{
  public:
	VGA()
	{}

	// TODO This function should be pure virtual, but it is currently implemented with added arguments in children classes
	virtual bool init(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{}

	virtual bool init(const Mode &mode, const PinConfig &pinConfig) = 0;

	int getCurrentLine(){ return currentLine; }

	Mode mode;

  protected:

	// This function represents the compromise to join a rendering engine with a canvas graphic buffer, and to pass geometry to the later
	virtual void propagateResolution(const int xres, const int yres) = 0;

	// Member variables related with the frame
	int totalLines;
	int currentLine;
	volatile bool vSyncPassed;

	// Functions and member variables related with sync bits
	virtual void initSyncBits() = 0;
	virtual long syncBits(bool h, bool v) = 0;
 
	int vsyncPin;
	int hsyncPin;
	long vsyncBit;
	long hsyncBit;
	long vsyncBitI;
	long hsyncBitI;
};
