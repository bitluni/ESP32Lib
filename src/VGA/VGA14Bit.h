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
#include "VGAI2SEngine.h"
#include "../Graphics/Graphics.h"
//#include "../Graphics/GraphicsR5G5B4S2Swapped.h"

class VGA14Bit : public VGAI2SEngine<BLpx1sz16sw1sh0>, public Graphics<ColorR5G5B4A2, BLpx1sz16sw1sh0, CTBIdentity>
{
  public:
	VGA14Bit(const int i2sIndex = 1)
		: VGAI2SEngine<BLpx1sz16sw1sh0>(i2sIndex)
	{
		frontColor = 0xffff;
	}

	bool init(Mode &mode,
			  const int R0Pin, const int R1Pin, const int R2Pin, const int R3Pin, const int R4Pin,
			  const int G0Pin, const int G1Pin, const int G2Pin, const int G3Pin, const int G4Pin,
			  const int B0Pin, const int B1Pin, const int B2Pin, const int B3Pin,
			  const int hsyncPin, const int vsyncPin, const int clockPin = -1)
	{
		const int bitCount = 16;
		int pinMap[bitCount] = {
			R0Pin, R1Pin, R2Pin, R3Pin, R4Pin,
			G0Pin, G1Pin, G2Pin, G3Pin, G4Pin,
			B0Pin, B1Pin, B2Pin, B3Pin,
			hsyncPin, vsyncPin
		};

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, const int clockPin = -1, const bool mostSignigicantPinFirst = false)
	{
		const int bitCount = 16;
		int pinMap[bitCount];
		for (int i = 0; i < 5; i++)
		{
			pinMap[i] = redPins[i];
			pinMap[i + 5] = greenPins[i];
			if (i < 4)
				pinMap[i + 10] = bluePins[i];
		}
		pinMap[14] = hsyncPin;
		pinMap[15] = vsyncPin;

		if(mostSignigicantPinFirst)
		{
			for (int i = 0; i < 5; i++)
			{
				pinMap[i] = redPins[4-i];
				pinMap[i + 5] = greenPins[4-i];
				if (i < 4)
					pinMap[i + 10] = bluePins[3-i];
			}
		}

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		const int bitCount = 16;
		int pinMap[bitCount];
		pinConfig.fill14Bit(pinMap);
		int clockPin = pinConfig.clock;

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	//THE REST OF THE FILE IS SHARED CODE BETWEEN 3BIT, 6BIT, AND 14BIT

	static const int bitMaskInRenderingBufferHSync()
	{
		return 1<<(8*bytesPerBufferUnit()-2);
	}

	static const int bitMaskInRenderingBufferVSync()
	{
		return 1<<(8*bytesPerBufferUnit()-1);
	}

	bool initoverlappingbuffers(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1)
	{
		lineBufferCount = mode.vRes / mode.vDiv; // yres
		rendererBufferCount = frameBufferCount;
		return initengine(mode, pinMap, bitCount, clockPin, 2); // 2 buffers per line
	}

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
