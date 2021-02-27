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


void IRAM_ATTR CompositeGrayLadderI::interrupt(void *arg)
{
	CompositeGrayLadderI * staticthis = (CompositeGrayLadderI *)arg;

	//obtain currently rendered line from the buffer just read, based on the conventioned ordering and buffers per line
	staticthis->currentLine = staticthis->dmaBufferDescriptorActive >> ( (staticthis->descriptorsPerLine==2) ? 1 : 0 );

	//in the case of two buffers per line,
	//render only when the sync half of the line ended (longer period until next interrupt)
	//else exit early
	//This might need to be revised, because it might be better to overlap and miss the second interrupt
	if ( (staticthis->descriptorsPerLine==2) && ((staticthis->dmaBufferDescriptorActive & 1) != 0) ) return;

	//render ahead (the lenght of buffered lines)
	int renderLine = (staticthis->currentLine + staticthis->lineBufferCount);
	if (renderLine >= staticthis->totalLines) renderLine -= staticthis->totalLines;

	if(!staticthis->mode.interlaced)
	{
		//TO DO: This should be precalculated outside the interrupt
		int vInactiveLinesCount = staticthis->mode.vFront + staticthis->mode.vOddFieldOffset + staticthis->mode.vBack;

		if (renderLine >= vInactiveLinesCount)
		{
			int renderActiveLine = renderLine - vInactiveLinesCount;
			uint8_t *activeRenderingBuffer = ((uint8_t *)
			staticthis->dmaBufferDescriptors[renderLine * staticthis->descriptorsPerLine + staticthis->descriptorsPerLine - 1].buffer() + staticthis->dataOffsetInLineInBytes
			);

			int y = renderActiveLine / staticthis->mode.vDiv;
			if (y >= 0 && y < staticthis->yres)
				staticthis->interruptPixelLine(y, activeRenderingBuffer, arg);
		}

		if (renderLine == 0)
			staticthis->vSyncPassed = true;
	} else {
		//TO DO: This should be precalculated outside the interrupt
		int oddFieldStart = staticthis->mode.vFront + staticthis->mode.vOddFieldOffset + staticthis->mode.vBack;
		int oddFieldEnd = oddFieldStart + staticthis->mode.vActive;
		int evenFieldStart = staticthis->mode.vFront + staticthis->mode.vEvenFieldOffset + staticthis->mode.vBack;
		int evenFieldEnd = evenFieldStart + staticthis->mode.vActive;
		
		if (renderLine >= oddFieldStart && renderLine < oddFieldEnd)
		{
			int renderActiveLine = renderLine - oddFieldStart;
			uint8_t *activeRenderingBuffer = ((uint8_t *)
			staticthis->dmaBufferDescriptors[renderLine * staticthis->descriptorsPerLine + staticthis->descriptorsPerLine - 1].buffer() + staticthis->dataOffsetInLineInBytes
			);

			int y = 2*renderActiveLine / staticthis->mode.vDiv;
			if (y >= 0 && y < staticthis->yres)
				staticthis->interruptPixelLine(y, activeRenderingBuffer, arg);
		} else if (renderLine >= evenFieldStart && renderLine < evenFieldEnd)
		{
			int renderActiveLine = renderLine - evenFieldStart;
			uint8_t *activeRenderingBuffer = ((uint8_t *)
			staticthis->dmaBufferDescriptors[renderLine * staticthis->descriptorsPerLine + staticthis->descriptorsPerLine - 1].buffer() + staticthis->dataOffsetInLineInBytes
			);

			int y = (2*renderActiveLine + 1) / staticthis->mode.vDiv;
			if (y >= 0 && y < staticthis->yres)
				staticthis->interruptPixelLine(y, activeRenderingBuffer, arg);
		}

		if (renderLine == 0)
			staticthis->vSyncPassed = true;
	}

}

void IRAM_ATTR CompositeGrayLadderI::interruptPixelLine(int y, uint8_t *pixels, void *arg)
{
	CompositeGrayLadderI * staticthis = (CompositeGrayLadderI *)arg;
	int p = staticthis->baseBufferValue << 8;
	int p0, p1, p2, p3;
	uint8_t *line = staticthis->frontBuffer[y];
	int j = 0;
	for (int i = 0; i < staticthis->mode.hRes / 4; i++)
	{
		//writing four pixels improves speed drastically (avoids memory reads)
		//instead of shifting, colorDepthConversionFactor was not divided by 256
		p0 = p1 = p2 = p3 = p;
		p0 += ((int)(line[j++]) * staticthis->colorDepthConversionFactor);
		p0 &= 0xff00;
		p1 += ((int)(line[j++]) * staticthis->colorDepthConversionFactor);
		p1 &= 0xff00;
		p2 += ((int)(line[j++]) * staticthis->colorDepthConversionFactor);
		p2 &= 0xff00;
		p3 += ((int)(line[j++]) * staticthis->colorDepthConversionFactor);
		p3 &= 0xff00;
		((uint32_t *)pixels)[i] = (p2 >> 8) | (p3 << 0) | (p0 << 8) | (p1 << 16);
	}
}
