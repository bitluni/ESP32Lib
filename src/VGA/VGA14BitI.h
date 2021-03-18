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
#include "VGAI2SDynamic.h"
#include "../Graphics/Graphics.h"

class VGA14BitI : public VGAI2SDynamic< BLpx1sz16sw1sh0, Graphics<ColorR5G5B4A2, BLpx1sz16sw0sh0, CTBIdentity> >
{
  public:
	VGA14BitI(const int i2sIndex = 1)
		: VGAI2SDynamic< BLpx1sz16sw1sh0, Graphics<ColorR5G5B4A2, BLpx1sz16sw0sh0, CTBIdentity> >(i2sIndex)
	{
		frontColor = 0xffff;
		interruptStaticChild = &VGA14BitI::interrupt;
	}

	bool init(const Mode &mode, 
		const int R0Pin, const int R1Pin, const int R2Pin, const int R3Pin, const int R4Pin,
		const int G0Pin, const int G1Pin, const int G2Pin, const int G3Pin, const int G4Pin,
		const int B0Pin, const int B1Pin, const int B2Pin, const int B3Pin, 
		const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		const int bitCount = 16;
		int pinMap[bitCount] = {
			R0Pin, R1Pin, R2Pin, R3Pin, R4Pin,
			G0Pin, G1Pin, G2Pin, G3Pin, G4Pin,
			B0Pin, B1Pin, B2Pin, B3Pin,
			hsyncPin, vsyncPin
		};
		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, const int clockPin = -1, const bool mostSignigicantPinFirst = false)
	{
		const int bitCount = 16;
		int pinMap[bitCount];
		for (int i = 0; i < 5; i++)
		{
			pinMap[i] = redPins[i];
			pinMap[i + 5] = greenPins[i];
			if (i < 4)
				pinMap[i + 10] = bluePins[i];
		}
		pinMap[14] = hsyncPin;
		pinMap[15] = vsyncPin;

		if(mostSignigicantPinFirst)
		{
			for (int i = 0; i < 5; i++)
			{
				pinMap[i] = redPins[4-i];
				pinMap[i + 5] = greenPins[4-i];
				if (i < 4)
					pinMap[i + 10] = bluePins[3-i];
			}
		}

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		const int bitCount = 16;
		int pinMap[bitCount];
		pinConfig.fill14Bit(pinMap);
		int clockPin = pinConfig.clock;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

  protected:
	static void interrupt(void *arg);

	static void interruptPixelLine(int y, uint8_t *pixels, void *arg);
};
