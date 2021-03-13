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
	
	   55 shades                  179 shades
	
	ESP32        TV           ESP32                       TV     
	-----+                     -----+    ____ 100 ohm
	    G|-                        G|---|____|+          
	pin25|--------- Comp       pin25|---|____|+--------- Comp    
	pin26|-                    pin26|-        220 ohm
	     |                          |
	     |                          |
	-----+                     -----+
	
	Connect pin 25 or 26
*/
#pragma once
#include "CompositeI2SEngine.h"
//#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsW8.h"


class CompositeGrayDACI : public CompositeI2SEngine<BLpx1sz16sw1sh8>, public GraphicsW8 // (=) Graphics<ColorW8, BLpx1sz8sw0sh0, CTBIdentity>
{
  public:
	CompositeGrayDACI() //DAC based modes only work with I2S0
		: CompositeI2SEngine<BLpx1sz16sw1sh8>(0)
	{
		colorMinValue = 23;
		syncLevel = 0;
		colorMaxValue = 77;
		interruptStaticChild = &CompositeGrayDACI::interrupt;
	}

	int outputPin = 25;
	bool voltageDivider = false;

	int colorDepthConversionFactor = 1;
	int colorMaxValue = 255;
	int colorMinValue = 77;

	bool init(const ModeComposite &mode, const int outputPin = 25, const bool voltageDivider = false)
	{
		const int bitCount = 16;
		int pinMap[bitCount] = {
			-1, -1, 
			-1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1,
			-1, -1, -1, -1
		};
		int clockPin = -1;
		this->outputPin = outputPin;
		this->voltageDivider = voltageDivider;
		if(voltageDivider)
		{
			colorMinValue = 77;
			syncLevel = 0;
			colorMaxValue = 255;
		}

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const ModeComposite &mode, const PinConfigComposite &pinConfig)
	{
		const int bitCount = 16;
		int pinMap[bitCount] = {
			-1, -1, 
			-1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1,
			-1, -1, -1, -1
		};
		int clockPin = -1;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool initengine(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin = -1, int descriptorsPerLine = 2)
	override
	{
		initenginePreparation(mode, pinMap, bitCount, clockPin, descriptorsPerLine);
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount, clockPin);
		enableDAC(outputPin==25?1:2);
		startTX();
		return true;
	}

	bool initdynamicwritetorenderbuffer(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{
		//values must be shifted to the MSByte to be output
		//which is equivalent to multiplying by 256
		//instead of shifting, do not divide here:
		//colorDepthConversionFactor = (colorMaxValue - colorMinValue + 1)/256;
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;

		baseBufferValue = colorMinValue;
		syncBufferValue = syncLevel;

		lineBufferCount = 3;
		rendererBufferCount = 1;
		return initengine(mode, pinMap, bitCount, clockPin, 1); // 1 buffer per line
	}

	//THE REST OF THE FILE IS SHARED CODE BETWEEN ...

	virtual void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
	}

	virtual void show(bool vSync = false)
	{
		if (!frameBufferCount)
			return;
		if (vSync)
		{
			vSyncPassed = false;
			while (!vSyncPassed)
				delay(0);
		}
		Graphics::show(vSync);
	}

  protected:
	bool useInterrupt()
	{ 
		return true; 
	};

	static void interrupt(void *arg);

	static void interruptPixelLine(int y, uint8_t *pixels, void *arg);
};
