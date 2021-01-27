/*
	Author: Martin-Laclaustra 2020
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
#include "../Graphics/GraphicsW8RangedSwapped.h"

class CompositeGrayLadder : public CompositeI2SEngine<BLpx1sz8sw2sh0>, public GraphicsW8RangedSwapped // (=) Graphics<ColorW8, BLpx1sz8sw2sh0, CTBRange>
{
  public:
	CompositeGrayLadder() //8 bit based modes only work with I2S1
		: CompositeI2SEngine<BLpx1sz8sw2sh0>(1)
	{
		colorMinValue = 76;
		syncLevel = 0;
		colorMaxValue = 255;
	}


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

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool init(const ModeComposite &mode, const int *pinMap)
	{
		const int bitCount = 8;
		int clockPin = -1;

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool init(const ModeComposite &mode, const PinConfigComposite &pinConfig)
	{
		const int bitCount = 8;
		int pinMap[bitCount];
		pinConfig.fill(pinMap);
		int clockPin = -1;

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool initoverlappingbuffers(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{
		//values must be divided to fit 8bits
		//instead of using a float, bitshift 8 bits to the right later:
		//colorDepthConversionFactor = (colorMaxValue - colorMinValue + 1)/256;
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;

		baseBufferValue = coltobuf(0,0,0);
		syncBufferValue = syncLevel;

		lineBufferCount = mode.vRes / mode.vDiv; // yres
		rendererBufferCount = frameBufferCount;
		return initengine(mode, pinMap, bitCount, clockPin, 2); // 2 buffers per line
	}

	//THE REST OF THE FILE IS SHARED CODE BETWEEN ...

	virtual void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
	}

	int currentBufferToAssign = 0;

	virtual BufferGraphicsUnit **allocateFrameBuffer()
	{
		void **arr = (void **)malloc(yres * sizeof(void *));
		if(!arr)
			ERROR("Not enough memory");
		for (int y = 0; y < yres; y++)
		{
			arr[y] = (void *)getBufferDescriptor(y, currentBufferToAssign);
		}
		currentBufferToAssign++;
		return (BufferGraphicsUnit **)arr;
	}

	virtual void show(bool vSync = false)
	{
		if (!frameBufferCount)
			return;

		Graphics::show(vSync);
		switchToRendererBuffer(currentFrameBuffer);
		// wait at least one frame
		// else the switch does not take place for the display
		// until the frame is completed
		// and drawing starts in the backbuffer while still shown
		if (frameBufferCount == 2) // in triple buffer or single buffer this is not an issue
		{
			uint32_t timemark = micros();
			uint32_t framedurationinus = (uint64_t)mode.pixelsPerLine() * (uint64_t)mode.linesPerField() * (uint64_t)1000000 / (uint64_t)mode.pixelClock;
			while((micros() - timemark) < framedurationinus){delay(0);}
		}
	}

	virtual void scroll(int dy, Color color)
	{
		Graphics::scroll(dy, color);
		if(dmaBufferDescriptors)
			for (int i = 0; i < yres * mode.vDiv; i++)
				if(!mode.interlaced || (i & 1) == 0) // odd line
				{
					dmaBufferDescriptors[
							indexRendererDataBuffer[(currentFrameBuffer + frameBufferCount - 1) % frameBufferCount]
							 + (i/(mode.interlaced?2:1)) * descriptorsPerLine + descriptorsPerLine - 1
						].setBuffer(
								((uint8_t *) backBuffer[i / mode.vDiv]) - dataOffsetInLineInBytes
								,
								((descriptorsPerLine > 1)?mode.hRes:mode.pixelsPerLine()) * bytesPerBufferUnit()/samplesPerBufferUnit()
						);
				} else { // even line
					dmaBufferDescriptors[
							indexRendererEvenDataBuffer[(currentFrameBuffer + frameBufferCount - 1) % frameBufferCount]
							 + ((i - 1)/2) * descriptorsPerLine + descriptorsPerLine - 1
						].setBuffer(
								((uint8_t *) backBuffer[i / mode.vDiv]) - dataOffsetInLineInBytes
								,
								((descriptorsPerLine > 1)?mode.hRes:mode.pixelsPerLine()) * bytesPerBufferUnit()/samplesPerBufferUnit()
						);
				}
	}
};
