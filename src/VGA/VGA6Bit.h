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
#include "VGAI2SOverlapping.h"
#include "../Graphics/Graphics.h"

class VGA6Bit : public VGAI2SOverlapping< BLpx1sz8sw2sh0, Graphics<ColorR2G2B2A2, BLpx1sz8sw2sh0, CTBIdentity> >
{
  public:
	VGA6Bit() //8 bit based modes only work with I2S1
		: VGAI2SOverlapping< BLpx1sz8sw2sh0, Graphics<ColorR2G2B2A2, BLpx1sz8sw2sh0, CTBIdentity> >(1)
	{
		frontColor = 0xff;
	}

	bool init(Mode &mode,
			  const int R0Pin, const int R1Pin,
			  const int G0Pin, const int G1Pin,
			  const int B0Pin, const int B1Pin,
			  const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		const int bitCount = 8;
		int pinMap[bitCount] = {
			R0Pin, R1Pin,
			G0Pin, G1Pin,
			B0Pin, B1Pin,
			hsyncPin, vsyncPin
		};

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, const int clockPin = -1, const bool mostSignigicantPinFirst = false)
	{
		const int bitCount = 8;
		int pinMap[bitCount];
		for (int i = 0; i < 2; i++)
		{
			pinMap[i] = redPins[i];
			pinMap[i + 2] = greenPins[i];
			pinMap[i + 4] = bluePins[i];
		}
		pinMap[6] = hsyncPin;
		pinMap[7] = vsyncPin;

		if(mostSignigicantPinFirst)
		{
			for (int i = 0; i < 2; i++)
			{
				pinMap[i] = redPins[1-i];
				pinMap[i + 2] = greenPins[1-i];
				pinMap[i + 4] = bluePins[1-i];
			}
		}

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		const int bitCount = 8;
		int pinMap[bitCount];
		pinConfig.fill6Bit(pinMap);
		int clockPin = pinConfig.clock;

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}
};
