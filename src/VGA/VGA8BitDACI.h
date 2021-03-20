/*
	Author: Martin-Laclaustra 2021
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/

/*
	CONNECTION
	
	A) voltageDivider = false; B) voltageDivider = true
	
	   55 shades                  255 shades
	
	ESP32        VGA           ESP32                       VGA     
	-----+                     -----+    ____ 100 ohm              
	    G|-   +---- R              G|---|____|+         +---- R    
	pin25|----+---- G          pin25|---|____|+---------+---- G    
	pin26|-   +---- B          pin26|-        220 ohm   +---- B    
	pin X|--------- HSYNC      pin X|------------------------ HSYNC
	pin Y|--------- VSYNC      pin Y|------------------------ VSYNC
	-----+                     -----+                              
	
	Connect pin 25 or 26
	Connect the 3 channels in parallel or whatever combination of them
	  depending on the monochrome color of choice
*/
#pragma once
#include "VGAI2SDynamic.h"
//#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsW8.h"


class VGA8BitDACI : public VGAI2SDynamic< BLpx1sz16sw1sh0, GraphicsW8 > // GraphicsW8 (=) Graphics<ColorW8, BLpx1sz8sw0sh0, CTBIdentity>
{
  public:
	VGA8BitDACI() //DAC based modes only work with I2S0
		: VGAI2SDynamic< BLpx1sz16sw1sh0, GraphicsW8 >(0)
	{
		frontColor = 0xff;
		colorMaxValue = 54;
		interruptStaticChild = &VGA8BitDACI::interrupt;
	}

	int outputPin = 25;
	bool voltageDivider = false;

	int colorDepthConversionFactor = 1;
	int colorMaxValue = 255;
	int colorMinValue = 0;

	bool init(const Mode &mode, const int hsyncPin, const int vsyncPin, const int outputPin = 25, const bool voltageDivider = false)
	{
		const int bitCount = 16;
		int pinMap[bitCount] = {
			-1, -1, -1, -1,
			-1, -1, hsyncPin, vsyncPin,
			-1, -1, -1, -1,
			-1, -1, -1, -1
		};
		int clockPin = -1;
		this->outputPin = outputPin;
		this->voltageDivider = voltageDivider;
		if(voltageDivider)
		{
			colorMaxValue = 255;
		}
		//values must be shifted to the MSByte to be output
		//which is equivalent to multiplying by 256
		//instead of shifting, do not divide here:
		//colorDepthConversionFactor = (colorMaxValue - colorMinValue + 1)/256;
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, const int clockPin = -1, const bool mostSignigicantPinFirst = false)
	{
		const int bitCount = 16;
		int pinMap[bitCount];
		this->hsyncPin = hsyncPin;
		this->vsyncPin = vsyncPin;
		for (int i = 0; i < 16; i++)
		{
			pinMap[i] = -1;
		}
		pinMap[6] = this->hsyncPin;
		pinMap[7] = this->vsyncPin;
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		const int bitCount = 16;
		int pinMap[bitCount];
		pinConfig.fill14Bit(pinMap);
		int clockPin = pinConfig.clock;
		this->hsyncPin = pinMap[14];
		this->vsyncPin = pinMap[15];
		for (int i = 0; i < 16; i++)
		{
			pinMap[i] = -1;
		}
		pinMap[6] = this->hsyncPin;
		pinMap[7] = this->vsyncPin;
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool initengine(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1, int descriptorsPerLine = 2)
	override
	{
		initenginePreparation(mode, pinMap, bitCount, clockPin, descriptorsPerLine);
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount, clockPin);
		enableDAC(outputPin==25?1:2); // this is added here to the initengine() base method
		startTX();
		return true;
	}

	const int bitMaskInRenderingBufferHSync()
	override
	{
		return 1<<(8*this->bytesPerBufferUnit()-2-8);
	}

	const int bitMaskInRenderingBufferVSync()
	override
	{
		return 1<<(8*this->bytesPerBufferUnit()-1-8);
	}

  protected:
	static void interrupt(void *arg);

	static void interruptPixelLine(int y, uint8_t *pixels, void *arg);
};
