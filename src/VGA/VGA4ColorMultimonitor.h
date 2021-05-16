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
#include "../Graphics/BufferLayouts/BLpx4sz16swmx1yshmxy.h"

class VGA4ColorMultimonitor : public VGAI2SOverlapping<BLpx1sz16sw1sh0, Graphics<ColorR1G1B1A1X4, BLpx4sz16swmx1yshmxy, CTBIdentity> >
{
  public:
	VGA4ColorMultimonitor(const int i2sIndex = 1)
		: VGAI2SOverlapping<BLpx1sz16sw1sh0, Graphics<ColorR1G1B1A1X4, BLpx4sz16swmx1yshmxy, CTBIdentity> >(i2sIndex)
	{
		frontColor = 0xf;
	}

	bool init(const Mode &mode,
			  const int R0Pin, const int G0Pin, const int B0Pin,
			  const int R1Pin, const int G1Pin, const int B1Pin,
			  const int R2Pin, const int G2Pin, const int B2Pin,
			  const int R3Pin, const int G3Pin, const int B3Pin,
			  const int R4Pin, const int G4Pin,
			  const int hsyncPin, const int vsyncPin, const int clockPin = -1,
			  const int horMonitorCount = 2, const int verMonitorCount = 2)
	{
		const int bitCount = 16;
		int pinMap[bitCount] = {
			R0Pin, G0Pin, B0Pin,
			R1Pin, G1Pin, B1Pin,
			R2Pin, G2Pin, B2Pin,
			R3Pin, G3Pin, B3Pin,
			-1, -1,
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

	bool init(const Mode &mode, const PinConfig &pinConfig, const int horMonitorCount = 2, const int verMonitorCount = 2)
	{
		const int bitCount = 16;
		int pinMap[bitCount];
		pinConfig.fill14Bit(pinMap);
		int clockPin = pinConfig.clock;
		mx = horMonitorCount;
		my = verMonitorCount;

		return initoverlappingbuffers(mode, pinMap, bitCount, clockPin);
	}

	bool initenginePreparation(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin, int descriptorsPerLine = 2)
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

	void clear(Color color = 0)
	override
	{
		BufferGraphicsUnit bufferUnaffectedBits = (backBuffer[0][0])&( vsyncBit | hsyncBit | vsyncBitI | hsyncBitI );
		BufferGraphicsUnit newColor = (BufferGraphicsUnit)( graphics_coltobuf(color & static_colormask(), 0, 0)*((0b001001001001) & (~( vsyncBit | hsyncBit | vsyncBitI | hsyncBitI ))) | bufferUnaffectedBits );
		for (int y = 0; y < this->wy; y++)
			for (int x = 0; x < this->wx; x++)
				backBuffer[y][x] = newColor;
	}
};
