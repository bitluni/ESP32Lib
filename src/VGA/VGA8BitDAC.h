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
#include "VGAI2SEngine.h"
//#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsX6S2W8RangedSwapped.h"


class VGA8BitDAC : public VGAI2SEngine<BLpx1sz16sw1sh0>, public GraphicsX6S2W8RangedSwapped // (=) Graphics<ColorW8, BLpx1sz16sw1sh8, CTBRange>
{
  public:
	VGA8BitDAC() //DAC based modes only work with I2S0
		: VGAI2SEngine<BLpx1sz16sw1sh0>(0)
	{
		frontColor = 0xff;
		colorMaxValue = 54;
	}

	int outputPin = 25;
	bool voltageDivider = false;

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

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
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

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
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

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
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


	static const int bitMaskInRenderingBufferHSync()
	{
		return 1<<(8*bytesPerBufferUnit()-2-8);
	}

	static const int bitMaskInRenderingBufferVSync()
	{
		return 1<<(8*bytesPerBufferUnit()-1-8);
	}

	bool initoverlappingbuffers(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{
		lineBufferCount = mode.vRes / mode.vDiv; // yres
		rendererBufferCount = frameBufferCount;
		return initengine(mode, pinMap, bitCount, clockPin, 2); // 2 buffers per line
	}

	//THE REST OF THE FILE IS SHARED CODE BETWEEN 3BIT, 6BIT, AND 14BIT

	virtual void initSyncBits()
	{
		hsyncBitI = mode.hSyncPolarity ? (bitMaskInRenderingBufferHSync()) : 0;
		vsyncBitI = mode.vSyncPolarity ? (bitMaskInRenderingBufferVSync()) : 0;
		hsyncBit = hsyncBitI ^ (bitMaskInRenderingBufferHSync());
		vsyncBit = vsyncBitI ^ (bitMaskInRenderingBufferVSync());
	}

	virtual long syncBits(bool hSync, bool vSync)
	{
		return ((hSync ? hsyncBit : hsyncBitI) | (vSync ? vsyncBit : vsyncBitI)) * rendererStaticReplicate32();
	}

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
				dmaBufferDescriptors[
						indexRendererDataBuffer[(currentFrameBuffer + frameBufferCount - 1) % frameBufferCount]
						 + i * descriptorsPerLine + descriptorsPerLine - 1
					].setBuffer(
							((uint8_t *) backBuffer[i / mode.vDiv]) - dataOffsetInLineInBytes
							,
							((descriptorsPerLine > 1)?mode.hRes:mode.pixelsPerLine()) * bytesPerBufferUnit()/samplesPerBufferUnit()
						);
	}
};
