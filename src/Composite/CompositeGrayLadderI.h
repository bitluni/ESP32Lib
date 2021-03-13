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
	
	A) R-2R resistor ladder; B) unequal rungs ladder
	
	   55 shades                  up to 254 shades?
	
	ESP32        TV           ESP32                       TV
	-----+                    -----+    ____ 
	    G|-+_____                 G|---|____|
	pinA0|-| R2R |- Comp      pinA0|---|____|+--------- Comp
	pinA1|-|     |            pinA1|---|____|
	pinA2|-|     |              ...|
	  ...|-|_____|                 |
	-----+                    -----+
	
	Connect pins of your choice (A0...A8=any pins).
	Custom ladders can be used by tweaking colorMinValue and colorMaxValue
*/
#pragma once
#include "CompositeI2SEngine.h"
//#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsW8.h"


class CompositeGrayLadderI : public CompositeI2SEngine<BLpx1sz8sw2sh0>, public GraphicsW8 // (=) Graphics<ColorW8, BLpx1sz8sw0sh0, CTBIdentity>
{
  public:
	CompositeGrayLadderI() //8 bit based modes only work with I2S1
		: CompositeI2SEngine<BLpx1sz8sw2sh0>(1)
	{
		colorMinValue = 76;
		syncLevel = 0;
		colorMaxValue = 255;
		interruptStaticChild = &CompositeGrayLadderI::interrupt;
	}

	int colorDepthConversionFactor = 1;
	int colorMaxValue = 255;
	int colorMinValue = 77;

	bool init(const ModeComposite &mode, 
			  const int C0Pin, const int C1Pin,
			  const int C2Pin, const int C3Pin,
			  const int C4Pin, const int C5Pin,
			  const int C6Pin, const int C7Pin)
	{
		const int bitCount = 8;
		int pinMap[bitCount] = {
			C0Pin, C1Pin,
			C2Pin, C3Pin,
			C4Pin, C5Pin,
			C6Pin, C7Pin
		};
		int clockPin = -1;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const ModeComposite &mode, const int *pinMap)
	{
		const int bitCount = 8;
		int clockPin = -1;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool init(const ModeComposite &mode, const PinConfigComposite &pinConfig)
	{
		const int bitCount = 8;
		int pinMap[bitCount];
		pinConfig.fill(pinMap);
		int clockPin = -1;

		return initdynamicwritetorenderbuffer(mode, pinMap, bitCount, clockPin);
	}

	bool initdynamicwritetorenderbuffer(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{
		//values must be divided to fit 8bits
		//instead of using a float, bitshift 8 bits to the right later:
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
