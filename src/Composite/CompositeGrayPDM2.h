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

    ESP32   ESP8266                                  TV     
    -----+  -----+
        G|--    G|-------------+-----------------+-- Comp (-)
         |       |        1 nF =                 |
         |       |   /   ____  |  ____     ____  |
     pinA|--   RX|-+' +-|____|-+-|____|-+-|____|-+
         |       |      22 ohm   47 ohm | 47 ohm
         |       |                      +---|(------ Comp
    -----+  -----+                       (+)10 uF

    Disconnect the ESP8266 circuit (switch) in order to reprogram (it blocks serial comm).
    ESP32 connect pin of your choice (A=any pin). Supply it as first pin in the pinMap.
*/
#pragma once
#include "CompositeI2SEngine.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/BufferLayouts/BLpx4sz8sw3xshx.h"
#include "../Graphics/ColorToBuffer/CTBRangePDM2.h"

class CompositeGrayPDM2 : public CompositeI2SEngine<BLpx4sz8sw3xshx>, public Graphics<ColorW8, BLpx4sz8sw3xshx, CTBRangePDM2>
{
  public:
	CompositeGrayPDM2() //8 bit based modes only work with I2S1
		: CompositeI2SEngine<BLpx4sz8sw3xshx>(1)
	{
		colorMinValue = 76;
		syncLevel = 0;
		colorMaxValue = 255;

		frontColor = 0xff;
		//defaultBufferValue = coltobuf(colorMinValue,0,0);
	}

	virtual void dotAdd(int x, int y, Color color)
	override
	{
		dot(x, y, color);
	}

	virtual void dotMix(int x, int y, Color color)
	override
	{
		dot(x, y, color);
	}


	bool init(const ModeComposite &mode, 
			  const int C0Pin, const int C1Pin,
			  const int C2Pin, const int C3Pin,
			  const int C4Pin, const int C5Pin,
			  const int C6Pin, const int C7Pin)
	{
		const int bitCount = 2;
		int pinMap[] = {
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
		const int bitCount = 2;
		int clockPin = -1;

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool init(const ModeComposite &mode, const PinConfigComposite &pinConfig)
	{
		const int bitCount = 2;
		int pinMap[bitCount];
		pinConfig.fill(pinMap);
		int clockPin = -1;

		bool result = initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
		DEBUG_PRINTLN("RETURNED FROM initoverlappingbuffers");

		//ESP8266 stalls unless some work is done here. Cause: unknown
		rect(30, 88, 255+5, 40+4, 127);
		for(int x = 0; x < 256; x++)
		{
			fillRect(x + 32, 90, 1, 40, x);
			if(x % 16 == 0)
			{
				fillRect(x + 32, 85, 1, 4, 255);
				setCursor(x + 32 - 3, 78);
			}
		}
		setCursor(0,0);
		clear();
		//End of code added to avoid stall.

		return result;
	}

	virtual bool initPDM(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin = -1, int descriptorsPerLine = 2)
	{
		this->mode = mode;
		int xres = mode.hRes;
		int yres = mode.vRes / mode.vDiv;
		totalLines = mode.linesPerFrame;
		if(descriptorsPerLine < 1 || descriptorsPerLine > 2) ERROR("Wrong number of descriptors per line");
		if(descriptorsPerLine == 1) allocateRendererBuffers1DescriptorsPerLine();
		if(descriptorsPerLine == 2) allocateRendererBuffers2DescriptorsPerLine();
		propagateResolution(xres, yres);
		//allocateLineBuffers();
		currentLine = 0;
		vSyncPassed = false;

		int factorBitCount = 1;
		if(bitCount < 8) factorBitCount = 8 / bitCount;

		initSerialOutputMode(pinMap[0], bitCount * factorBitCount, -1, clockPin, mode.pixelClock / factorBitCount);
		startTX();
		return true;
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
		DEBUG_PRINTLN("PRE INIT ENGINE");
		bool result = initPDM(mode, pinMap, bitCount, clockPin, 2); // 2 buffers per line
		DEBUG_PRINTLN("POST INIT ENGINE");
		return result;
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
