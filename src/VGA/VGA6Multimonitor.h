/*
	Author: Martin-Laclaustra 2021
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once
#include "VGAI2SOverlapping.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/BufferLayouts/BLpx6sz8swmx2yshmxy.h"

class VGA6Multimonitor : public VGAI2SOverlapping< BLpx1sz8sw2sh0, Graphics<ColorW1X7, BLpx6sz8swmx2yshmxy, CTBIdentity> >
{
  public:
	VGA6Multimonitor() //8 bit based modes only work with I2S1
		: VGAI2SOverlapping< BLpx1sz8sw2sh0, Graphics<ColorW1X7, BLpx6sz8swmx2yshmxy, CTBIdentity> >(1)
	{
		frontColor = 0x1;
	}

	bool init(const Mode &mode,
			  const int M0Pin, const int M1Pin,
			  const int M2Pin, const int M3Pin,
			  const int M4Pin, const int M5Pin,
			  const int hsyncPin, const int vsyncPin, const int clockPin = -1,
			  const int horMonitorCount = 3, const int verMonitorCount = 2)
	{
		const int bitCount = 8;
		int pinMap[bitCount] = {
			M0Pin, M1Pin,
			M2Pin, M3Pin,
			M4Pin, M5Pin,
			hsyncPin, vsyncPin
		};
		mx = horMonitorCount;
		my = verMonitorCount;

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		return init(mode, pinConfig, 3, 2);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig, const int horMonitorCount = 3, const int verMonitorCount = 2)
	{
		const int bitCount = 8;
		int pinMap[bitCount];
		pinConfig.fill6Bit(pinMap);
		int clockPin = pinConfig.clock;
		mx = horMonitorCount;
		my = verMonitorCount;

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool initenginePreparation(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1, int descriptorsPerLine = 2)
	override
	{
		this->mode = mode;
		int xres = mx * mode.hRes; // This line changes from VGAI2SEngine
		int yres = my * mode.vRes / mode.vDiv; // This line changes from VGAI2SEngine
		wx = mode.hRes; // This line is added to VGAI2SEngine
		wy = mode.vRes / mode.vDiv; // This line is added to VGAI2SEngine
		initSyncBits();
		this->vsyncPin = pinMap[8*bytesPerBufferUnit()-1];
		this->hsyncPin = pinMap[8*bytesPerBufferUnit()-2];
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

	void scroll(int dy, Color color)
	override
	{
		Graphics::scroll(dy, color);
		if(this->dmaBufferDescriptors)
			for (int i = 0; i < this->yres * this->mode.vDiv / this->my; i++)
				this->dmaBufferDescriptors[
						this->indexRendererDataBuffer[(this->currentFrameBuffer + this->frameBufferCount - 1) % this->frameBufferCount]
						 + i * this->descriptorsPerLine + this->descriptorsPerLine - 1
					].setBuffer(
							((uint8_t *) this->backBuffer[i / this->mode.vDiv]) - this->dataOffsetInLineInBytes
							,
							((this->descriptorsPerLine > 1)?this->mode.hRes:this->mode.pixelsPerLine()) * this->bytesPerBufferUnit()/this->samplesPerBufferUnit()
						);
	}
};
