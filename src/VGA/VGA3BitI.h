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
#include "../Graphics/GraphicsR1G1B1A1.h"

class VGA3BitI : public VGAI2SDynamic< BLpx1sz8sw2sh0, GraphicsR1G1B1A1 > // GraphicsR1G1B1A1 (=) public Graphics<ColorR1G1B1A1X4, BLpx2sz8swxshx, CTBIdentity>
{
  public:
	VGA3BitI() //8 bit based modes only work with I2S1
		: VGAI2SDynamic< BLpx1sz8sw2sh0, GraphicsR1G1B1A1 >(1)
	{
		frontColor = 0xf;
		interruptStaticChild = &VGA3BitI::interrupt;
	}

	bool init(const Mode &mode, const int RPin, const int GPin, const int BPin, const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		const int bitCount = 8;
		int pinMap[bitCount] = {
			RPin,
			GPin,
			BPin,
			-1, -1, -1,
			hsyncPin, vsyncPin
		};
		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, const int clockPin = -1, const bool mostSignigicantPinFirst = false)
	{
		const int bitCount = 8;
		int pinMap[bitCount];
		for (int i = 0; i < 8; i++)
		{
			pinMap[i] = -1;
		}
		pinMap[0] = redPins[0];
		pinMap[1] = greenPins[0];
		pinMap[2] = bluePins[0];
		pinMap[6] = hsyncPin;
		pinMap[7] = vsyncPin;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		const int bitCount = 8;
		int pinMap[bitCount];
		pinConfig.fill3Bit(pinMap);
		int clockPin = pinConfig.clock;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

  protected:
	static void interrupt(void *arg);

	static void interruptPixelLine(int y, uint8_t *pixels, void *arg);
};
