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
#include "../Graphics/GraphicsX6S2W8RangedSwapped.h"


class CompositeGrayDAC : public CompositeI2SEngine<BLpx1sz16sw1sh8>, public GraphicsX6S2W8RangedSwapped // (=) Graphics<ColorW8, BLpx1sz16sw1sh8, CTBRange>
{
  public:
	CompositeGrayDAC() //DAC based modes only work with I2S0
		: CompositeI2SEngine<BLpx1sz16sw1sh8>(0)
	{
		colorMinValue = 23;
		syncLevel = 0;
		colorMaxValue = 77;
	}

	int outputPin = 25;
	bool voltageDivider = false;

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

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
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

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool initengine(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin = -1, int descriptorsPerLine = 2)
	override
	{
		initenginePreparation(mode, pinMap, bitCount, clockPin, descriptorsPerLine);
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount, clockPin);
		enableDAC(outputPin==25?1:2); // this is added here to the initengine() base method
		startTX();
		return true;
	}

	bool initoverlappingbuffers(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{
		//values must be shifted to the MSByte to be output
		//which is equivalent to multiplying by 256
		//instead of shifting, do not divide here:
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
