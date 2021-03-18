/*
	Author: Martin-Laclaustra 2021
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
*/
#pragma once
#include "VGAI2SDynamic.h"
#include "../Graphics/GraphicsTextBuffer.h"

class VGATextI : public VGAI2SDynamic< BLpx1sz8sw2sh0, GraphicsTextBuffer > // GraphicsTextBuffer (=) Graphics<ColorW8, BLpx1sz8sw0sh0, CTBIdentity>
{
  public:
	VGATextI() //8 bit based modes only work with I2S1
		: VGAI2SDynamic< BLpx1sz8sw2sh0, GraphicsTextBuffer >(1)
	{
		frontColor = 0xf;
		interruptStaticChild = &VGATextI::interrupt;
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

	bool initenginePreparation(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin, int descriptorsPerLine = 1)
	override
	{
		if (!font)
			return false;
		this->mode = mode;
		int xres = (mode.hRes + font->charWidth - 1) / font->charWidth;
		int yres = ((mode.vRes / mode.vDiv) + font->charHeight - 1) / font->charHeight;
		initSyncBits();
		this->vsyncPin = vsyncPin;
		this->hsyncPin = hsyncPin;
		totalLines = mode.linesPerField();
		if(descriptorsPerLine < 1 || descriptorsPerLine > 2) ERROR("Wrong number of descriptors per line");
		if(descriptorsPerLine == 1) allocateRendererBuffers1DescriptorsPerLine();
		if(descriptorsPerLine == 2) allocateRendererBuffers2DescriptorsPerLine();
		propagateResolution(xres, yres);
		//allocateLineBuffers();
		currentLine = 0;
		vSyncPassed = false;
		return true;
	}

  protected:
	static void interrupt(void *arg);

	static void interruptPixelLine(int y, uint8_t *pixels, void *arg);
};
