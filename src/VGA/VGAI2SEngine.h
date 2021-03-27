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

#include "VGA.h"
#include "../I2S/I2S.h"

#include "../Tools/Log.h"

template<class BufferLayout>
class VGAI2SEngine : public I2S, public VGA, public BufferLayout
{
  public:
	VGAI2SEngine(const int i2sIndex = 0)
	: I2S(i2sIndex)
	{
		dmaBufferDescriptors = 0; // I2S member variable
		lineBufferCount = 1;
		rendererBufferCount = 1;
		rendererStaticReplicate32mask = rendererStaticReplicate32();
	}

	virtual bool initenginePreparation(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1, int descriptorsPerLine = 2)
	{
		this->mode = mode;
		int xres = mode.hRes;
		int yres = mode.vRes / mode.vDiv;
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

	virtual bool initengine(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1, int descriptorsPerLine = 2)
	{
		initenginePreparation(mode, pinMap, bitCount, clockPin, descriptorsPerLine);
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount, clockPin);
		startTX();
		return true;
	}


	// Templated definitions and functions to pass to children (might this work with alias or using?)

	typedef typename BufferLayout::BufferUnit BufferRendererUnit;

	static int bytesPerBufferUnit()
	{
		return sizeof(BufferRendererUnit);
	}
	static int samplesPerBufferUnit()
	{
		return BufferLayout::static_xpixperunit();
	}
	static int renderer_xpixperunit()
	{
		return BufferLayout::static_xpixperunit();
	}
	static int renderer_ypixperunit()
	{
		return BufferLayout::static_ypixperunit();
	}
	static int renderer_replicate()
	{
		return BufferLayout::static_replicate();
	}
	static int rendererStaticReplicate32() // TODO Refactor to renderer_replicate32()
	{
		return BufferLayout::static_replicate32();
	}
	static int renderer_swx(int x)
	{
		return BufferLayout::static_swx(x);
	}
	static int renderer_swy(int y)
	{
		return BufferLayout::static_swy(y);
	}
	static int renderer_shval(BufferRendererUnit val, int x, int y)
	{
		return BufferLayout::static_shval(val, x, y);
	}
	static int renderer_shbuf(BufferRendererUnit val, int x, int y)
	{
		return BufferLayout::static_shbuf(val, x, y);
	}


	// Functions related with sync bits

	virtual const int bitMaskInRenderingBufferHSync()
	{
		return 1<<(8*this->bytesPerBufferUnit()-2);
	}

	virtual const int bitMaskInRenderingBufferVSync()
	{
		return 1<<(8*this->bytesPerBufferUnit()-1);
	}

	virtual void initSyncBits()
	{
		this->hsyncBitI = this->mode.hSyncPolarity ? (this->bitMaskInRenderingBufferHSync()) : 0;
		this->vsyncBitI = this->mode.vSyncPolarity ? (this->bitMaskInRenderingBufferVSync()) : 0;
		this->hsyncBit = this->hsyncBitI ^ (this->bitMaskInRenderingBufferHSync());
		this->vsyncBit = this->vsyncBitI ^ (this->bitMaskInRenderingBufferVSync());
	}

	virtual long syncBits(bool hSync, bool vSync)
	{
		return ((hSync ? this->hsyncBit : this->hsyncBitI) | (vSync ? this->vsyncBit : this->vsyncBitI)) * this->rendererStaticReplicate32();
	}


	// Member variables specific to this engine

	int lineBufferCount;
	int rendererBufferCount;
	int indexRendererDataBuffer[3];
	int indexHingeDataBuffer; // last fixed buffer that "jumps" to the active data buffer

	int baseBufferValue = 0;

	int descriptorsPerLine = 0;
	int dataOffsetInLineInBytes = 0;

	int rendererStaticReplicate32mask = 0;


	// Functions specific to this engine

	void setLineBufferCount(int lineBufferCount)
	{
		this->lineBufferCount = lineBufferCount;
	}

	BufferRendererUnit * getBufferDescriptor(int y, int bufferIndex = 0)
	{
		return (BufferRendererUnit *) (dmaBufferDescriptors[indexRendererDataBuffer[bufferIndex] + y*mode.vDiv * descriptorsPerLine + descriptorsPerLine - 1].buffer() + dataOffsetInLineInBytes);
	}

	void switchToRendererBuffer(int bufferNumber)
	{
		dmaBufferDescriptors[indexHingeDataBuffer].next(dmaBufferDescriptors[indexRendererDataBuffer[bufferNumber]]);
	}

	void dump()
	{
		DEBUG_PRINTLN("");
		DEBUG_PRINTLN("=================================================");
		DEBUG_PRINTLN("");

		int byteCounter = 0;

		for (int i = 0; i < dmaBufferDescriptorCount; i++)
		{
			uint8_t* currentDMABuffer = (uint8_t*)dmaBufferDescriptors[i].buffer();
			int currentDMABufferSize = dmaBufferDescriptors[i].getSize();
			for (int j = 0; j < currentDMABufferSize; j++)
			{
				if(byteCounter++ % 32 == 0) DEBUG_PRINT("\n");
				DEBUG_PRINTF(currentDMABuffer[j],HEX);
				DEBUG_PRINT(" ");
			}
			delay(0);
		}

		DEBUG_PRINTLN("");
		DEBUG_PRINTLN("=================================================");
		DEBUG_PRINTLN("");
	}

	//complete ringbuffer for frame
	void allocateRendererBuffers2DescriptorsPerLine()
	{
		descriptorsPerLine = 2;
		//determine parameters
		//lenght of each line
		int samplesHLineComplete = mode.hFront + mode.hSync + mode.hBack + mode.hRes;
		int samplesHBlanking = mode.hFront + mode.hSync + mode.hBack;
		int samplesHData = mode.hRes;

		int sizeHLineComplete = samplesHLineComplete * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHLineCompleteAligned32 = (sizeHLineComplete + 3) & 0xfffffffc;
		int sizeHBlanking = samplesHBlanking * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHBlankingAligned32 = (sizeHBlanking + 3) & 0xfffffffc;
		int sizeHData = samplesHData * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHDataAligned32 = (sizeHData + 3) & 0xfffffffc;

		dataOffsetInLineInBytes = 0;

		//videolines and data videolines for each frame
		int linesTotal = mode.vFront + mode.vSync + mode.vBack + mode.vRes;
		int dataLinesBufferCount = lineBufferCount;

		//calculate DMA buffer descriptors needed
		dmaBufferDescriptorCount = linesTotal * descriptorsPerLine;
		//account for more descriptors for additional backbuffers
		if (rendererBufferCount > 1) dmaBufferDescriptorCount += (rendererBufferCount - 1) * mode.vRes * descriptorsPerLine;
		//allocate DMA buffer descriptors for the whole frame
		dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
		//link all buffer descriptors in a ring
		for (int i = 0; i < dmaBufferDescriptorCount; i++)
			dmaBufferDescriptors[i].next(dmaBufferDescriptors[(i + 1) % dmaBufferDescriptorCount]);
		//close the ring at the appropriate descriptors in case there are additional backbuffers
		//and record the position of descriptors for the data part of the buffer
		for (int b = 0; b < rendererBufferCount; b++)
		{
			indexRendererDataBuffer[b] = (mode.vFront + mode.vSync + mode.vBack) * descriptorsPerLine + b * mode.vRes * descriptorsPerLine;
			dmaBufferDescriptors[indexRendererDataBuffer[b] + mode.vRes * descriptorsPerLine - 1].next(dmaBufferDescriptors[0]);
		}
		indexHingeDataBuffer = (mode.vFront + mode.vSync + mode.vBack) * descriptorsPerLine - 1;



		//create and fill the buffers with their default values

		void *vBlankingHBlankingBuffer;
		void *vBlankingHDataBuffer;
		void *vSyncHBlankingBuffer;
		void *vSyncHDataBuffer;
		void **DataBuffer; // vDataHDataBuffer

		//create the buffers
		//1 blank prototype line for vFront and vBack
		vBlankingHBlankingBuffer = DMABufferDescriptor::allocateBuffer(sizeHBlankingAligned32, true);
		vBlankingHDataBuffer = DMABufferDescriptor::allocateBuffer(sizeHDataAligned32, true);
		//1 sync prototype line for vSync
		vSyncHBlankingBuffer = DMABufferDescriptor::allocateBuffer(sizeHBlankingAligned32, true);
		vSyncHDataBuffer = DMABufferDescriptor::allocateBuffer(sizeHDataAligned32, true);
		//n lines as buffer for data lines
		//allocated elsewhere (actually below)
		DataBuffer = (void **)malloc(rendererBufferCount * dataLinesBufferCount * sizeof(void *));
		for (int i = 0; i < rendererBufferCount * dataLinesBufferCount; i++)
		{
			DataBuffer[i] = DMABufferDescriptor::allocateBuffer(sizeHDataAligned32, true);
		}
		//create a live-refill buffer (when dataLinesBufferCount != mode.vRes/mode.vDiv)
		// or create a whole buffer, but duplicating lines according to vDiv


		//fill the buffers with their default values
		for (int i = 0; i < samplesHBlanking; i++)
		{
			if (i >= mode.hFront && i < mode.hFront + mode.hSync)
			{
				//delete old data
				((BufferRendererUnit *)vSyncHBlankingBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				((BufferRendererUnit *)vBlankingHBlankingBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)vSyncHBlankingBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval((baseBufferValue | hsyncBit | vsyncBit)&BufferLayout::static_bufferdatamask(), i, 0);
				((BufferRendererUnit *)vBlankingHBlankingBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval((baseBufferValue | hsyncBit | vsyncBitI)&BufferLayout::static_bufferdatamask(), i, 0);
			}
			else
			{
				//delete old data
				((BufferRendererUnit *)vSyncHBlankingBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				((BufferRendererUnit *)vBlankingHBlankingBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)vSyncHBlankingBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval((baseBufferValue | hsyncBitI | vsyncBit)&BufferLayout::static_bufferdatamask(), i, 0);
				((BufferRendererUnit *)vBlankingHBlankingBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval((baseBufferValue | hsyncBitI | vsyncBitI)&BufferLayout::static_bufferdatamask(), i, 0);
			}
		}
		for (int i = 0; i < samplesHData; i++)
		{
			//delete old data
			((BufferRendererUnit *)vSyncHDataBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
			((BufferRendererUnit *)vBlankingHDataBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
			//set new data
			((BufferRendererUnit *)vSyncHDataBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval((baseBufferValue | hsyncBitI | vsyncBit)&BufferLayout::static_bufferdatamask(), i, 0);
			((BufferRendererUnit *)vBlankingHDataBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval((baseBufferValue | hsyncBitI | vsyncBitI)&BufferLayout::static_bufferdatamask(), i, 0);
		}

		for (int i = 0; i < rendererBufferCount * dataLinesBufferCount; i++)
		{
			memcpy(DataBuffer[i], vBlankingHDataBuffer, sizeHDataAligned32);
		}



		//assign the buffers accross the DMA buffer descriptors
		//CONVENTION: the frame starts after the last active (data) line of previous frame
		//CONVENTION: the line starts after the last active (data) sample of previous line
		int d = 0;
		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingHBlankingBuffer, sizeHBlanking);
			dmaBufferDescriptors[d++].setBuffer(vBlankingHDataBuffer, sizeHData);
		}
		for (int i = 0; i < mode.vSync; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vSyncHBlankingBuffer, sizeHBlanking);
			dmaBufferDescriptors[d++].setBuffer(vSyncHDataBuffer, sizeHData);
		}
		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingHBlankingBuffer, sizeHBlanking);
			dmaBufferDescriptors[d++].setBuffer(vBlankingHDataBuffer, sizeHData);
		}
		for (int b = 0; b < rendererBufferCount; b++)
		{
			for (int i = 0; i < mode.vRes; i++)
			{
				dmaBufferDescriptors[d++].setBuffer(vBlankingHBlankingBuffer, sizeHBlanking);
				dmaBufferDescriptors[d++].setBuffer(DataBuffer[b * dataLinesBufferCount + (i/mode.vDiv) % dataLinesBufferCount], sizeHData);
			}
		}

		free(DataBuffer);
	}


	void allocateRendererBuffers1DescriptorsPerLine()
	{
		descriptorsPerLine = 1;
		//determine parameters
		//lenght of each line
		int samplesHLineComplete = mode.hFront + mode.hSync + mode.hBack + mode.hRes;
		int samplesHBlanking = mode.hFront + mode.hSync + mode.hBack;
		int samplesHData = mode.hRes;

		int sizeHLineComplete = samplesHLineComplete * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHLineCompleteAligned32 = (sizeHLineComplete + 3) & 0xfffffffc;
		int sizeHBlanking = samplesHBlanking * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHBlankingAligned32reduced = (sizeHBlanking) & 0xfffffffc;

		dataOffsetInLineInBytes = sizeHBlankingAligned32reduced;

		//videolines and data videolines for each frame
		int linesTotal = mode.vFront + mode.vSync + mode.vBack + mode.vRes;
		int dataLinesBufferCount = lineBufferCount;

		//calculate DMA buffer descriptors needed
		dmaBufferDescriptorCount = linesTotal * descriptorsPerLine;
		//account for more descriptors for additional backbuffers
		if (rendererBufferCount > 1) dmaBufferDescriptorCount += (rendererBufferCount - 1) * mode.vRes * descriptorsPerLine;
		//allocate DMA buffer descriptors for the whole frame
		dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
		//link all buffer descriptors in a ring
		for (int i = 0; i < dmaBufferDescriptorCount; i++)
			dmaBufferDescriptors[i].next(dmaBufferDescriptors[(i + 1) % dmaBufferDescriptorCount]);
		//close the ring at the appropriate descriptors in case there are additional backbuffers
		//and record the position of descriptors for the data part of the buffer
		for (int b = 0; b < rendererBufferCount; b++)
		{
			indexRendererDataBuffer[b] = (mode.vFront + mode.vSync + mode.vBack) * descriptorsPerLine + b * mode.vRes * descriptorsPerLine;
			dmaBufferDescriptors[indexRendererDataBuffer[b] + mode.vRes * descriptorsPerLine - 1].next(dmaBufferDescriptors[0]);
		}
		indexHingeDataBuffer = (mode.vFront + mode.vSync + mode.vBack) * descriptorsPerLine - 1;



		//create and fill the buffers with their default values

		void *vBlankingLineBuffer;
		void *vSyncLineBuffer;
		void **DataBuffer; // vDataLineBuffer

		//create the buffers
		//1 blank prototype line for vFront and vBack
		vBlankingLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
		//1 sync prototype line for vSync
		vSyncLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
		//n lines as buffer for data lines
		//allocated elsewhere (actually below)
		DataBuffer = (void **)malloc(rendererBufferCount * dataLinesBufferCount * sizeof(void *));
		for (int i = 0; i < rendererBufferCount * dataLinesBufferCount; i++)
		{
			DataBuffer[i] = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
		}
		//create a live-refill buffer (when dataLinesBufferCount != mode.vRes/mode.vDiv)
		// or create a whole buffer, but duplicating lines according to vDiv


		//fill the buffers with their default values
		for (int i = 0; i < samplesHLineComplete; i++)
		{
			if (i >= mode.hFront && i < mode.hFront + mode.hSync)
			{
				//delete old data
				((BufferRendererUnit *)vSyncLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				((BufferRendererUnit *)vBlankingLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)vSyncLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval((baseBufferValue | hsyncBit | vsyncBit)&BufferLayout::static_bufferdatamask(), i, 0);
				((BufferRendererUnit *)vBlankingLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval((baseBufferValue | hsyncBit | vsyncBitI)&BufferLayout::static_bufferdatamask(), i, 0);
			}
			else
			{
				//delete old data
				((BufferRendererUnit *)vSyncLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				((BufferRendererUnit *)vBlankingLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)vSyncLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval((baseBufferValue | hsyncBitI | vsyncBit)&BufferLayout::static_bufferdatamask(), i, 0);
				((BufferRendererUnit *)vBlankingLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval((baseBufferValue | hsyncBitI | vsyncBitI)&BufferLayout::static_bufferdatamask(), i, 0);
			}
		}

		for (int i = 0; i < rendererBufferCount * dataLinesBufferCount; i++)
		{
			memcpy(DataBuffer[i], vBlankingLineBuffer, sizeHLineCompleteAligned32);
		}



		//assign the buffers accross the DMA buffer descriptors
		//CONVENTION: the frame starts after the last active (data) line of previous frame
		//CONVENTION: the line starts after the last active (data) sample of previous line
		int d = 0;
		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingLineBuffer, sizeHLineComplete);
		}
		for (int i = 0; i < mode.vSync; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vSyncLineBuffer, sizeHLineComplete);
		}
		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingLineBuffer, sizeHLineComplete);
		}
		for (int b = 0; b < rendererBufferCount; b++)
		{
			for (int i = 0; i < mode.vRes; i++)
			{
				dmaBufferDescriptors[d++].setBuffer(DataBuffer[b * dataLinesBufferCount + (i/mode.vDiv) % dataLinesBufferCount], sizeHLineComplete);
			}
		}

		free(DataBuffer);
	}

};
