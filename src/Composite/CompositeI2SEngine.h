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

#include "Composite.h"

#include "../Tools/Log.h"

template<class BufferLayout>
class CompositeI2SEngine : public Composite, public BufferLayout
{
  public:
	CompositeI2SEngine(const int i2sIndex = 0)
	: Composite(i2sIndex)
	{
		lineBufferCount = 1;
		dmaBufferDescriptors = 0;
		rendererBufferCount = 1;
		rendererStaticReplicate32mask = rendererStaticReplicate32();
	}

	//stump to fullfil requirement implementation from parent Composite class
	virtual int bytesPerSample() const { return 0; }

	//stump to fullfil requirement implementation from parent Composite class
	virtual void allocateLineBuffers() {}

	virtual bool initenginePreparation(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin = -1, int descriptorsPerLine = 2)
	{
		this->mode = mode;
		int xres = mode.hRes;
		int yres = mode.vRes / mode.vDiv;
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

	virtual bool initengine(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin = -1, int descriptorsPerLine = 2)
	{
		initenginePreparation(mode, pinMap, bitCount, clockPin, descriptorsPerLine);
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount, clockPin);
		startTX();
		return true;
	}

	typedef typename BufferLayout::BufferUnit BufferRendererUnit;

	//pass templated functions
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


	//members for multiple buffers (at the renderer "side")
	//used in DMA-based (no interrupt-based) modes
	int rendererBufferCount; // number of buffers (1-3)
	int indexRendererDataBuffer[3]; // index of the first DMA descriptor of each buffer (or its odd field in interlaced modes)
	int indexHingeDataBuffer; // last common DMA descriptor that "jumps" to the first DMA descriptor of active data buffer (or its odd field)

	int indexLandingFromOddDataBuffer; // first common DMA descriptor that "receives" from the last DMA descriptor of active (or odd) data buffer (0 in non-interlaced modes) (Landing is always 0 for the even field)
	int indexRendererEvenDataBuffer[3]; // index of the first DMA descriptor of each even field buffer
	int indexHingeEvenDataBuffer; // last common DMA descriptor that "jumps" to the first DMA descriptor of the even field active data buffer

	//other members
	int baseBufferValue = 0;
	int syncBufferValue = 0;

	int descriptorsPerLine = 0;
	int dataOffsetInLineInBytes = 0;

	int rendererStaticReplicate32mask = 0;

	static int bytesPerBufferUnit()
	{
		return sizeof(BufferRendererUnit);
	}
	static int samplesPerBufferUnit()
	{
		return BufferLayout::static_xpixperunit();
	}
	static int rendererStaticReplicate32()
	{
		return BufferLayout::static_replicate32();
	}

	BufferRendererUnit * getBufferDescriptor(int y, int bufferIndex = 0)
	{
		if(!mode.interlaced || ((y*mode.vDiv) & 1) == 0) // odd line
		{
			return (BufferRendererUnit *) (dmaBufferDescriptors[indexRendererDataBuffer[bufferIndex] + ((y*mode.vDiv)/(mode.interlaced?2:1)) * descriptorsPerLine + descriptorsPerLine - 1].buffer() + dataOffsetInLineInBytes);
			//return (BufferRendererUnit *) (dmaBufferDescriptors[(mode.vFront + mode.vOddFieldOffset + mode.vBack) * descriptorsPerLine + ((y*mode.vDiv)/(mode.interlaced?2:1)) * descriptorsPerLine + descriptorsPerLine - 1].buffer() + dataOffsetInLineInBytes);
		} else { // even line
			return (BufferRendererUnit *) (dmaBufferDescriptors[indexRendererEvenDataBuffer[bufferIndex] + ((y*mode.vDiv - 1)/2) * descriptorsPerLine + descriptorsPerLine - 1].buffer() + dataOffsetInLineInBytes);
			//return (BufferRendererUnit *) (dmaBufferDescriptors[(mode.vFront + mode.vEvenFieldOffset + mode.vBack) * descriptorsPerLine + ((y*mode.vDiv - 1)/2) * descriptorsPerLine + descriptorsPerLine - 1].buffer() + dataOffsetInLineInBytes);
		}
		//THIS MUST BE FIXED FOR INTERLACED MODES
		//return (BufferRendererUnit *) (dmaBufferDescriptors[indexRendererDataBuffer[bufferIndex] + y*mode.vDiv * descriptorsPerLine + descriptorsPerLine - 1].buffer() + dataOffsetInLineInBytes);
	}

	void switchToRendererBuffer(int bufferNumber)
	{
		//THIS MUST BE FIXED FOR INTERLACED MODES
		dmaBufferDescriptors[indexHingeDataBuffer].next(dmaBufferDescriptors[indexRendererDataBuffer[bufferNumber]]);
		if (mode.interlaced)
			dmaBufferDescriptors[indexHingeEvenDataBuffer].next(dmaBufferDescriptors[indexRendererEvenDataBuffer[bufferNumber]]);
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
		//vsync lines
		int samplesHalfLine = samplesHLineComplete/2;
		// Line must have an even number of samples


		int sizeHLineComplete = samplesHLineComplete * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHLineCompleteAligned32 = (sizeHLineComplete + 3) & 0xfffffffc;
		int sizeHBlanking = samplesHBlanking * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHBlankingAligned32 = (sizeHBlanking + 3) & 0xfffffffc;
		int sizeHData = samplesHData * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHDataAligned32 = (sizeHData + 3) & 0xfffffffc;
		//int sizeHLineBundledCompleteAligned32 = sizeHBlankingAligned32 + sizeHDataAligned32;
		int sizeHalfLine = samplesHalfLine * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHalfLineAligned32 = (sizeHalfLine + 3) & 0xfffffffc;

		dataOffsetInLineInBytes = 0;

		//videolines and data videolines for each frame
		int linesTotal = mode.linesPerFrame; // this accounts for interlaced frames (2 fields)
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
		//
		//IMPLEMENTATION FOR MULTIPLE BUFFERS
		//WARNING: FOR INTERLACED MODES THERE ARE TWO BIFURCATIONS AND TWO REENTRY POINTS
		//THIS MAY BE COMPLEX TO IMPLEMENT - CURRENLY EXPERIMENTAL
		//
		//close the ring at the appropriate descriptors in case there are additional backbuffers
		//and record the position of descriptors for the data part of the buffer
		//even additional data buffer descriptors will be located after odd additional data buffer descriptors
		indexLandingFromOddDataBuffer = mode.interlaced?((mode.vFront + mode.vOddFieldOffset + mode.vBack + mode.vActive) * descriptorsPerLine):0;
		indexRendererDataBuffer[0] = (mode.vFront + mode.vOddFieldOffset + mode.vBack) * descriptorsPerLine + 0 * mode.vActive * descriptorsPerLine;
		dmaBufferDescriptors[indexRendererDataBuffer[0] + mode.vActive * descriptorsPerLine - 1].next(dmaBufferDescriptors[indexLandingFromOddDataBuffer]);
		if(mode.interlaced)
		{
			indexRendererEvenDataBuffer[0] = (mode.vFront + mode.vEvenFieldOffset + mode.vBack) * descriptorsPerLine + 0 * mode.vActive * descriptorsPerLine;
			dmaBufferDescriptors[indexRendererEvenDataBuffer[0] + mode.vActive * descriptorsPerLine - 1].next(dmaBufferDescriptors[0]);
		}
		for (int b = 1; b < rendererBufferCount; b++)
		{
			indexRendererDataBuffer[b] = linesTotal * descriptorsPerLine + (b - 1) * mode.vActive * descriptorsPerLine;
			dmaBufferDescriptors[indexRendererDataBuffer[b] + mode.vActive * descriptorsPerLine - 1].next(dmaBufferDescriptors[indexLandingFromOddDataBuffer]);
			if(mode.interlaced)
			{
				indexRendererEvenDataBuffer[b] = linesTotal * descriptorsPerLine + (rendererBufferCount - 1 + (b - 1)) * mode.vActive * descriptorsPerLine;
				dmaBufferDescriptors[indexRendererEvenDataBuffer[b] + mode.vActive * descriptorsPerLine - 1].next(dmaBufferDescriptors[0]);
			}
		}
		indexHingeDataBuffer = (mode.vFront + mode.vOddFieldOffset + mode.vBack) * descriptorsPerLine - 1;
		indexHingeEvenDataBuffer = (mode.vFront + mode.vEvenFieldOffset + mode.vBack) * descriptorsPerLine - 1;



		//create and fill the buffers with their default values

		//void *vBlankingHBlankingBuffer;
		void *vBlankingHDataBuffer;

		//Vertical sync
		//4 possible Half Lines (HL):
		// NormalFront (NF), NormalBack (NB), Equalizing (EQ), Sync (SY)
		void *normalFrontHalfLineBuffer;
		void *equalizingHalfLineBuffer;
		void *vSyncHalfLineBuffer;
		void *normalBackHalfLineBuffer;

		void **DataBuffer; // vDataHDataBuffer

		//create the buffers
		//1 blank prototype line for vFront and vBack
		//vBlankingHBlankingBuffer = DMABufferDescriptor::allocateBuffer(sizeHBlankingAligned32, true);
		vBlankingHDataBuffer = DMABufferDescriptor::allocateBuffer(sizeHDataAligned32, true);
		//1 prototype for each HL type in vSync
		equalizingHalfLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHalfLineAligned32, true);
		vSyncHalfLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHalfLineAligned32, true);
		normalFrontHalfLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHalfLineAligned32, true);
		normalBackHalfLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHalfLineAligned32, true);
		//overlapping buffers for space saving
		//vBlankingHBlankingBuffer = normalFrontHalfLineBuffer;
		//normalBackHalfLineBuffer = vBlankingHDataBuffer;
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
		//for (int i = 0; i < samplesHBlanking; i++)
		//{
			//if (i >= mode.hFront && i < mode.hFront + mode.hSync)
			//{
				////delete old data
				//((BufferRendererUnit *)vBlankingHBlankingBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				////set new data
				//((BufferRendererUnit *)vBlankingHBlankingBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
			//}
			//else
			//{
				////delete old data
				//((BufferRendererUnit *)vBlankingHBlankingBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				////set new data
				//((BufferRendererUnit *)vBlankingHBlankingBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(baseBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
			//}
		//}
		for (int i = 0; i < samplesHData; i++)
		{
			//delete old data
			((BufferRendererUnit *)vBlankingHDataBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
			//set new data
			((BufferRendererUnit *)vBlankingHDataBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(baseBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
		}
		// hFront + hSync should never be bigger than HalfLine or this will not work
		for (int i = 0; i < samplesHalfLine; i++)
		{
			//equalizingHalfLineBuffer
			if (i >= mode.hFront && i < mode.hFront + mode.hSync/2)
			{
				//delete old data
				((BufferRendererUnit *)equalizingHalfLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)equalizingHalfLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
			}
			else
			{
				//delete old data
				((BufferRendererUnit *)equalizingHalfLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)equalizingHalfLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(baseBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
			}
			//vSyncHalfLineBuffer
			if (i >= mode.hFront && i < mode.hFront + (samplesHalfLine - mode.hSync))
			{
				//delete old data
				((BufferRendererUnit *)vSyncHalfLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)vSyncHalfLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
			}
			else
			{
				//delete old data
				((BufferRendererUnit *)vSyncHalfLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)vSyncHalfLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(baseBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
			}
			//normalFrontHalfLineBuffer
			if (i >= mode.hFront && i < mode.hFront + mode.hSync)
			{
				//delete old data
				((BufferRendererUnit *)normalFrontHalfLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)normalFrontHalfLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
			}
			else
			{
				//delete old data
				((BufferRendererUnit *)normalFrontHalfLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)normalFrontHalfLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(baseBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
			}
			//normalBackHalfLineBuffer
			//delete old data
			((BufferRendererUnit *)normalBackHalfLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
			//set new data
			((BufferRendererUnit *)normalBackHalfLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(baseBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
		}

		for (int i = 0; i < rendererBufferCount * dataLinesBufferCount; i++)
		{
			memcpy(DataBuffer[i], vBlankingHDataBuffer, sizeHDataAligned32);
		}



		//assign the buffers accross the DMA buffer descriptors
		//CONVENTION: the frame starts after the last active line of previous frame
		//CONVENTION: the line starts after the last active (data) sample of previous line
		int d = 0;
		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHalfLine);
			dmaBufferDescriptors[d++].setBuffer(normalBackHalfLineBuffer, sizeHalfLine);
		}
		//pre-line
		int consumelines = mode.vOPreRegHL;
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHalfLine);
			dmaBufferDescriptors[d++].setBuffer(normalBackHalfLineBuffer, sizeHalfLine);
			consumelines-=2;
		}
		//NF
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHalfLine);
		}
		//EQ
		consumelines = mode.vPreEqHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(equalizingHalfLineBuffer, sizeHalfLine);
		//SY
		consumelines = mode.vSyncHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(vSyncHalfLineBuffer, sizeHalfLine);
		//EQ
		consumelines = mode.vPostEqHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(equalizingHalfLineBuffer, sizeHalfLine);
		//NB
		consumelines = mode.vOPostRegHL;
		if((consumelines & 1) == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(normalBackHalfLineBuffer, sizeHalfLine);
			consumelines--;
		}
		//post-line
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHalfLine);
			dmaBufferDescriptors[d++].setBuffer(normalBackHalfLineBuffer, sizeHalfLine);
			consumelines-=2;
		}

		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHalfLine);
			dmaBufferDescriptors[d++].setBuffer(normalBackHalfLineBuffer, sizeHalfLine);
		}
		// for (int b = 0; b < rendererBufferCount; b++)
		for (int i = 0; i < mode.vActive; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHBlanking);
			int b = 0; // multiple buffers do not work at the moment
			dmaBufferDescriptors[d++].setBuffer(DataBuffer[b * dataLinesBufferCount + ((i*(mode.interlaced?2:1)) / mode.vDiv) % dataLinesBufferCount], sizeHData);
		}

		// here d should be linesPerFrame*2 if mode is progressive
		// and linesPerFrame*2 / 2 if mode is interlaced
		if(mode.interlaced)
		{

		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHalfLine);
			dmaBufferDescriptors[d++].setBuffer(normalBackHalfLineBuffer, sizeHalfLine);
		}
		//pre-line
		int consumelines = mode.vEPreRegHL;
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHalfLine);
			dmaBufferDescriptors[d++].setBuffer(normalBackHalfLineBuffer, sizeHalfLine);
			consumelines-=2;
		}
		//NF
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHalfLine);
		}
		//EQ
		consumelines = mode.vPreEqHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(equalizingHalfLineBuffer, sizeHalfLine);
		//SY
		consumelines = mode.vSyncHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(vSyncHalfLineBuffer, sizeHalfLine);
		//EQ
		consumelines = mode.vPostEqHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(equalizingHalfLineBuffer, sizeHalfLine);
		//NB
		consumelines = mode.vEPostRegHL;
		if((consumelines & 1) == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(normalBackHalfLineBuffer, sizeHalfLine);
			consumelines--;
		}
		//post-line
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHalfLine);
			dmaBufferDescriptors[d++].setBuffer(normalBackHalfLineBuffer, sizeHalfLine);
			consumelines-=2;
		}

		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHalfLine);
			dmaBufferDescriptors[d++].setBuffer(normalBackHalfLineBuffer, sizeHalfLine);
		}
		// for (int b = 0; b < rendererBufferCount; b++)
		for (int i = 0; i < mode.vActive; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHBlanking);
			int b = 0; // multiple buffers do not work at the moment
			dmaBufferDescriptors[d++].setBuffer(DataBuffer[b * dataLinesBufferCount + ((i*2 + 1) / mode.vDiv) % dataLinesBufferCount], sizeHData);
		}

		}

		//assign additional (or odd field) buffers
		for (int b = 1; b < rendererBufferCount; b++)
		{
			for (int i = 0; i < mode.vActive; i++)
			{
				dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHBlanking);
				dmaBufferDescriptors[d++].setBuffer(DataBuffer[b * dataLinesBufferCount + ((i*(mode.interlaced?2:1)) / mode.vDiv) % dataLinesBufferCount], sizeHData);
			}
		}
		if(mode.interlaced)
		{
		//assign additional even field buffers
		for (int b = 1; b < rendererBufferCount; b++)
		{
			for (int i = 0; i < mode.vActive; i++)
			{
				dmaBufferDescriptors[d++].setBuffer(normalFrontHalfLineBuffer, sizeHBlanking);
				dmaBufferDescriptors[d++].setBuffer(DataBuffer[b * dataLinesBufferCount + ((i*2 + 1) / mode.vDiv) % dataLinesBufferCount], sizeHData);
			}
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
		//vsync lines
		int samplesHalfLine = samplesHLineComplete/2;
		// Line must have an even number of samples


		int sizeHLineComplete = samplesHLineComplete * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHLineCompleteAligned32 = (sizeHLineComplete + 3) & 0xfffffffc;
		int sizeHBlanking = samplesHBlanking * bytesPerBufferUnit()/samplesPerBufferUnit();
		int sizeHBlankingAligned32reduced = (sizeHBlanking) & 0xfffffffc;
		int sizeHalfLine = samplesHalfLine * bytesPerBufferUnit()/samplesPerBufferUnit();

		dataOffsetInLineInBytes = sizeHBlankingAligned32reduced;

		//videolines and data videolines for each frame
		int linesTotal = mode.linesPerFrame; // this accounts for interlaced frames (2 fields)
		int dataLinesBufferCount = lineBufferCount;

		//calculate DMA buffer descriptors needed
		dmaBufferDescriptorCount = linesTotal * descriptorsPerLine;
		//account for more descriptors for additional backbuffers
		if (rendererBufferCount > 1) dmaBufferDescriptorCount += (rendererBufferCount - 1) * mode.vRes * descriptorsPerLine; // this will not work for interlaced frames (then, use only 1 buffer)
		//allocate DMA buffer descriptors for the whole frame
		dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
		//link all buffer descriptors in a ring
		for (int i = 0; i < dmaBufferDescriptorCount; i++)
			dmaBufferDescriptors[i].next(dmaBufferDescriptors[(i + 1) % dmaBufferDescriptorCount]);
		//
		//IMPLEMENTATION FOR MULTIPLE BUFFERS
		//WARNING: FOR INTERLACED MODES THERE ARE TWO BIFURCATIONS AND TWO REENTRY POINTS
		//THIS MAY BE COMPLEX TO IMPLEMENT - CURRENLY EXPERIMENTAL
		//
		//close the ring at the appropriate descriptors in case there are additional backbuffers
		//and record the position of descriptors for the data part of the buffer
		//even additional data buffer descriptors will be located after odd additional data buffer descriptors
		indexLandingFromOddDataBuffer = mode.interlaced?((mode.vFront + mode.vOddFieldOffset + mode.vBack + mode.vActive) * descriptorsPerLine):0;
		indexRendererDataBuffer[0] = (mode.vFront + mode.vOddFieldOffset + mode.vBack) * descriptorsPerLine + 0 * mode.vActive * descriptorsPerLine;
		dmaBufferDescriptors[indexRendererDataBuffer[0] + mode.vActive * descriptorsPerLine - 1].next(dmaBufferDescriptors[indexLandingFromOddDataBuffer]);
		if(mode.interlaced)
		{
			indexRendererEvenDataBuffer[0] = (mode.vFront + mode.vEvenFieldOffset + mode.vBack) * descriptorsPerLine + 0 * mode.vActive * descriptorsPerLine;
			dmaBufferDescriptors[indexRendererEvenDataBuffer[0] + mode.vActive * descriptorsPerLine - 1].next(dmaBufferDescriptors[0]);
		}
		for (int b = 1; b < rendererBufferCount; b++)
		{
			indexRendererDataBuffer[b] = linesTotal * descriptorsPerLine + (b - 1) * mode.vActive * descriptorsPerLine;
			dmaBufferDescriptors[indexRendererDataBuffer[b] + mode.vActive * descriptorsPerLine - 1].next(dmaBufferDescriptors[indexLandingFromOddDataBuffer]);
			if(mode.interlaced)
			{
				indexRendererEvenDataBuffer[b] = linesTotal * descriptorsPerLine + (rendererBufferCount - 1 + (b - 1)) * mode.vActive * descriptorsPerLine;
				dmaBufferDescriptors[indexRendererEvenDataBuffer[b] + mode.vActive * descriptorsPerLine - 1].next(dmaBufferDescriptors[0]);
			}
		}
		indexHingeDataBuffer = (mode.vFront + mode.vOddFieldOffset + mode.vBack) * descriptorsPerLine - 1;
		indexHingeEvenDataBuffer = (mode.vFront + mode.vEvenFieldOffset + mode.vBack) * descriptorsPerLine - 1;



		//create and fill the buffers with their default values

		void *vBlankingLineBuffer;

		//Vertical sync
		//4 possible Half Lines (HL):
		// NormalFront (NF), NormalBack (NB), Equalizing (EQ), Sync (SY)
		//6 possible Full Lines (plus one complete normal line)
		void *normalFrontEqualizingLineBuffer;
		void *equalizingEqualizingLineBuffer;
		void *equalizingVSyncLineBuffer;
		void *vSyncVSyncLineBuffer;
		void *vSyncEqualizingLineBuffer;
		void *equalizingNormalBackLineBuffer;

		//If sizeHLineComplete is divisible by 8:
		//All can be obtained as pointers on a single concatenated one
		void *vSyncPrototypesBuffer;
		//NF-EQ-EQ-SY-SY-EQ-NB
		//^  ^  ^  ^  ^  ^

		void **DataBuffer; // vDataHDataBuffer

		//create the buffers
		//1 blank prototype line for vFront and vBack
		vBlankingLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
		if(sizeHLineComplete % 8 == 0) // divisible by 8
		{
			//1 prototype for all vSync
			vSyncPrototypesBuffer = DMABufferDescriptor::allocateBuffer(sizeHalfLine * 7, true);
			normalFrontEqualizingLineBuffer = (void*)&(((uint8_t*)vSyncPrototypesBuffer)[0 * sizeHalfLine]);
			equalizingEqualizingLineBuffer = (void*)&(((uint8_t*)vSyncPrototypesBuffer)[1 * sizeHalfLine]);
			equalizingVSyncLineBuffer = (void*)&(((uint8_t*)vSyncPrototypesBuffer)[2 * sizeHalfLine]);
			vSyncVSyncLineBuffer = (void*)&(((uint8_t*)vSyncPrototypesBuffer)[3 * sizeHalfLine]);
			vSyncEqualizingLineBuffer = (void*)&(((uint8_t*)vSyncPrototypesBuffer)[4 * sizeHalfLine]);
			equalizingNormalBackLineBuffer = (void*)&(((uint8_t*)vSyncPrototypesBuffer)[5 * sizeHalfLine]);
		} else {
			//1 prototype for each vSync
			normalFrontEqualizingLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
			equalizingEqualizingLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
			equalizingVSyncLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
			vSyncVSyncLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
			vSyncEqualizingLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
			equalizingNormalBackLineBuffer = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
		}
		//n lines as buffer for active lines
		//allocated elsewhere (actually below)
		DataBuffer = (void **)malloc(rendererBufferCount * dataLinesBufferCount * sizeof(void *));
		for (int i = 0; i < rendererBufferCount * dataLinesBufferCount; i++)
		{
			DataBuffer[i] = DMABufferDescriptor::allocateBuffer(sizeHLineCompleteAligned32, true);
		}
		//create a live-refill buffer (when dataLinesBufferCount != mode.vRes/mode.vDiv)
		// or create a whole buffer, but duplicating lines according to vDiv

		//fill the buffers with their default values
		//blank line
		for (int i = 0; i < samplesHLineComplete; i++)
		{
			//delete old data
			((BufferRendererUnit *)vBlankingLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
			//set new data
			((BufferRendererUnit *)vBlankingLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(baseBufferValue & BufferLayout::static_bufferdatamask(), i, 0);
		}
		memcpy(normalFrontEqualizingLineBuffer, vBlankingLineBuffer, sizeHLineCompleteAligned32);
		memcpy(equalizingEqualizingLineBuffer, vBlankingLineBuffer, sizeHLineCompleteAligned32);
		memcpy(equalizingVSyncLineBuffer, vBlankingLineBuffer, sizeHLineCompleteAligned32);
		memcpy(vSyncVSyncLineBuffer, vBlankingLineBuffer, sizeHLineCompleteAligned32);
		memcpy(vSyncEqualizingLineBuffer, vBlankingLineBuffer, sizeHLineCompleteAligned32);
		memcpy(equalizingNormalBackLineBuffer, vBlankingLineBuffer, sizeHLineCompleteAligned32);
		//vSyncPrototypesBuffer;
		// hFront + hSync should never be bigger than HalfLine or this will not work

		//Second halves first
		for (int i = 0; i < samplesHalfLine; i++)
		{
		//NF-EQ-EQ-SY-SY-EQ-NB
		//^  ^  ^  ^  ^  ^
		//(NF)-EQ-EQ-SY-SY-EQ-NB
		//Nothing needed. Just the blank level (set above)

		//NF-(EQ)-(EQ)-SY-SY-(EQ)-NB
			if (i >= mode.hFront && i < mode.hFront + mode.hSync/2)
			{

				//delete old data
				((BufferRendererUnit *)normalFrontEqualizingLineBuffer)[BufferLayout::static_swx(samplesHalfLine+i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), samplesHalfLine+i, 0);
				//set new data
				((BufferRendererUnit *)normalFrontEqualizingLineBuffer)[BufferLayout::static_swx(samplesHalfLine+i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), samplesHalfLine+i, 0);

				//delete old data
				((BufferRendererUnit *)equalizingEqualizingLineBuffer)[BufferLayout::static_swx(samplesHalfLine+i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), samplesHalfLine+i, 0);
				//set new data
				((BufferRendererUnit *)equalizingEqualizingLineBuffer)[BufferLayout::static_swx(samplesHalfLine+i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), samplesHalfLine+i, 0);

				//delete old data
				((BufferRendererUnit *)vSyncEqualizingLineBuffer)[BufferLayout::static_swx(samplesHalfLine+i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), samplesHalfLine+i, 0);
				//set new data
				((BufferRendererUnit *)vSyncEqualizingLineBuffer)[BufferLayout::static_swx(samplesHalfLine+i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), samplesHalfLine+i, 0);

			}
		//NF-EQ-EQ-(SY)-(SY)-EQ-NB
			if (i >= mode.hFront && i < mode.hFront + (samplesHalfLine - mode.hSync))
			{

				//delete old data
				((BufferRendererUnit *)equalizingVSyncLineBuffer)[BufferLayout::static_swx(samplesHalfLine+i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), samplesHalfLine+i, 0);
				//set new data
				((BufferRendererUnit *)equalizingVSyncLineBuffer)[BufferLayout::static_swx(samplesHalfLine+i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), samplesHalfLine+i, 0);

				//delete old data
				((BufferRendererUnit *)vSyncVSyncLineBuffer)[BufferLayout::static_swx(samplesHalfLine+i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), samplesHalfLine+i, 0);
				//set new data
				((BufferRendererUnit *)vSyncVSyncLineBuffer)[BufferLayout::static_swx(samplesHalfLine+i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), samplesHalfLine+i, 0);

			}
		//NF-EQ-EQ-SY-SY-EQ-(NB)
		//Nothing needed. Just the blank level (set above)
		}

		//First halves later
		for (int i = 0; i < samplesHalfLine; i++)
		{
		//NF-EQ-EQ-SY-SY-EQ-NB
		//^  ^  ^  ^  ^  ^
		//(NF)-EQ-EQ-SY-SY-EQ-NB
			if (i >= mode.hFront && i < mode.hFront + mode.hSync)
			{

				//delete old data
				((BufferRendererUnit *)vBlankingLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)vBlankingLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);

				//delete old data
				((BufferRendererUnit *)normalFrontEqualizingLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)normalFrontEqualizingLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);

			}
		//NF-(EQ)-(EQ)-SY-SY-(EQ)-NB
			if (i >= mode.hFront && i < mode.hFront + mode.hSync/2)
			{

				//delete old data
				((BufferRendererUnit *)equalizingEqualizingLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)equalizingEqualizingLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);

				//delete old data
				((BufferRendererUnit *)equalizingVSyncLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)equalizingVSyncLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);

				//delete old data
				((BufferRendererUnit *)equalizingNormalBackLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)equalizingNormalBackLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);

			}
		//NF-EQ-EQ-(SY)-(SY)-EQ-NB
			if (i >= mode.hFront && i < mode.hFront + (samplesHalfLine - mode.hSync))
			{

				//delete old data
				((BufferRendererUnit *)vSyncVSyncLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)vSyncVSyncLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);

				//delete old data
				((BufferRendererUnit *)vSyncEqualizingLineBuffer)[BufferLayout::static_swx(i)] &= ~BufferLayout::static_shval(BufferLayout::static_bufferdatamask(), i, 0);
				//set new data
				((BufferRendererUnit *)vSyncEqualizingLineBuffer)[BufferLayout::static_swx(i)] |= BufferLayout::static_shval(syncBufferValue & BufferLayout::static_bufferdatamask(), i, 0);

			}
		//NF-EQ-EQ-SY-SY-EQ-(NB)
		//Nothing needed. Just the blank level (set above)
		}

		for (int i = 0; i < rendererBufferCount * dataLinesBufferCount; i++)
		{
			memcpy(DataBuffer[i], vBlankingLineBuffer, sizeHLineCompleteAligned32);
		}



		//assign the buffers accross the DMA buffer descriptors
		//CONVENTION: the frame starts after the last active line of previous frame
		//CONVENTION: the line starts after the last active (data) sample of previous line
		int d = 0;
		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingLineBuffer, sizeHLineComplete);
		}
		//pre-line
		int consumelines = mode.vOPreRegHL;
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingLineBuffer, sizeHLineComplete);
			consumelines-=2;
		}
		//NF-EQ
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontEqualizingLineBuffer, sizeHLineComplete);
			consumelines = mode.vPreEqHL - 1;
		} else {
			consumelines = mode.vPreEqHL;
		}
		//EQ-EQ
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(equalizingEqualizingLineBuffer, sizeHLineComplete);
			consumelines-=2;
		}
		//EQ-SY
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(equalizingVSyncLineBuffer, sizeHLineComplete);
			consumelines = mode.vSyncHL - 1;
		} else {
			consumelines = mode.vSyncHL;
		}
		//SY-SY
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(vSyncVSyncLineBuffer, sizeHLineComplete);
			consumelines-=2;
		}
		//SY-EQ
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(vSyncEqualizingLineBuffer, sizeHLineComplete);
			consumelines = mode.vPostEqHL - 1;
		} else {
			consumelines = mode.vPostEqHL;
		}
		//EQ-EQ
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(equalizingEqualizingLineBuffer, sizeHLineComplete);
			consumelines-=2;
		}
		//EQ-NB
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(equalizingNormalBackLineBuffer, sizeHLineComplete);
			consumelines = mode.vOPostRegHL - 1;
		} else {
			consumelines = mode.vOPostRegHL;
		}
		//post-line
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingLineBuffer, sizeHLineComplete);
			consumelines-=2;
		}

		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingLineBuffer, sizeHLineComplete);
		}
		// for (int b = 0; b < rendererBufferCount; b++)
		for (int i = 0; i < mode.vActive; i++)
		{
			int b = 0; // multiple buffers do not work at the moment
			dmaBufferDescriptors[d++].setBuffer(DataBuffer[b * dataLinesBufferCount + ((i*(mode.interlaced?2:1)) / mode.vDiv) % dataLinesBufferCount], sizeHLineComplete);
		}

		//DEBUG_PRINTLN(d);
		// here d should be linesPerFrame if mode is progressive
		// and ~linesPerFrame / 2 if mode is interlaced
		if(mode.interlaced)
		{

		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingLineBuffer, sizeHLineComplete);
		}
		//pre-line
		int consumelines = mode.vEPreRegHL;
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingLineBuffer, sizeHLineComplete);
			consumelines-=2;
		}
		//NF-EQ
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontEqualizingLineBuffer, sizeHLineComplete);
			consumelines = mode.vPreEqHL - 1;
		} else {
			consumelines = mode.vPreEqHL;
		}
		//EQ-EQ
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(equalizingEqualizingLineBuffer, sizeHLineComplete);
			consumelines-=2;
		}
		//EQ-SY
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(equalizingVSyncLineBuffer, sizeHLineComplete);
			consumelines = mode.vSyncHL - 1;
		} else {
			consumelines = mode.vSyncHL;
		}
		//SY-SY
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(vSyncVSyncLineBuffer, sizeHLineComplete);
			consumelines-=2;
		}
		//SY-EQ
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(vSyncEqualizingLineBuffer, sizeHLineComplete);
			consumelines = mode.vPostEqHL - 1;
		} else {
			consumelines = mode.vPostEqHL;
		}
		//EQ-EQ
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(equalizingEqualizingLineBuffer, sizeHLineComplete);
			consumelines-=2;
		}
		//EQ-NB
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(equalizingNormalBackLineBuffer, sizeHLineComplete);
			consumelines = mode.vEPostRegHL - 1;
		} else {
			consumelines = mode.vEPostRegHL;
		}
		//post-line
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingLineBuffer, sizeHLineComplete);
			consumelines-=2;
		}

		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(vBlankingLineBuffer, sizeHLineComplete);
		}
		// for (int b = 0; b < rendererBufferCount; b++)
		for (int i = 0; i < mode.vActive; i++)
		{
			int b = 0; // multiple buffers do not work at the moment
			dmaBufferDescriptors[d++].setBuffer(DataBuffer[b * dataLinesBufferCount + ((i*2 + 1) / mode.vDiv) % dataLinesBufferCount], sizeHLineComplete);
		}

		}

		//assign additional (or odd field) buffers
		for (int b = 1; b < rendererBufferCount; b++)
		{
			for (int i = 0; i < mode.vActive; i++)
			{
				dmaBufferDescriptors[d++].setBuffer(DataBuffer[b * dataLinesBufferCount + ((i*(mode.interlaced?2:1)) / mode.vDiv) % dataLinesBufferCount], sizeHLineComplete);
			}
		}
		if(mode.interlaced)
		{
		//assign additional even field buffers
		for (int b = 1; b < rendererBufferCount; b++)
		{
			for (int i = 0; i < mode.vActive; i++)
			{
				dmaBufferDescriptors[d++].setBuffer(DataBuffer[b * dataLinesBufferCount + ((i*2 + 1) / mode.vDiv) % dataLinesBufferCount], sizeHLineComplete);
			}
		}
		}

		free(DataBuffer);
	}

};
