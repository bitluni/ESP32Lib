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
	
	   55 shades                  145 shades
	
	ESP32        TV           ESP32                       TV     
	-----+                     -----+    ____ 100 ohm
	    G|-                        G|---|____|+          
	pin25|--------- Comp       pin25|---|____|+--------- Comp    
	pin26|-                    pin26|-        150 ohm
	     |                          |
	     |                          |
	-----+                     -----+                              
	
	Connect pin 25 or 26
*/
#pragma once
#include "Composite.h"
#include "../Graphics/GraphicsM8CA8Swapped.h"


class CompositeColorDACMemory : public Composite, public GraphicsM8CA8Swapped
{
  public:
	CompositeColorDACMemory() //DAC based modes only work with I2S0
		: Composite(0)
	{
		lineBufferCount = 3;
		//Raw DAC output values
		//---------------------
		levelHighClipping = 95;
		levelWhite = 77;
		amplitudeBurst = 11;
		levelBlack = 23;
		levelBlanking = 23;
		levelLowClipping = 5;
		levelSync = 0;
		//Normalized (with voltage divider) DAC output values
		//---------------------
		//levelHighClipping = 255;
		//levelWhite = 207;
		//amplitudeBurst = 31;
		//levelBlack = 62;
		//levelBlanking = 62
		//levelLowClipping = 14;
		//levelSync = 0;
	}

	bool init(const ModeComposite &mode, const int outputPin = 25, const bool voltageDivider = false)
	{
		int pinMap[16] = {
			-1, -1, 
			-1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1,
			-1, -1, -1, -1
		};
		this->outputPin = outputPin;
		this->voltageDivider = voltageDivider;
		if(voltageDivider)
		{
			levelHighClipping = 255;
			levelWhite = 207;
			amplitudeBurst = 31;
			levelBlack = 62;
			levelBlanking = 62;
			levelLowClipping = 14;
			levelSync = 0;
		}
		firstPixelOffset = mode.hSync + mode.hBack;
		colorClock0x1000Periods = 0x1000L*(float)mode.pixelClock/mode.colorClock; // pixels per 0x1000 color cycles
		bufferVDiv = mode.vDiv;
		bufferInterlaced = mode.interlaced;
		bufferPhaseAlternating = mode.phaseAlternating;
		return initDAC(mode, pinMap, 16, -1);
	}

	bool init(const ModeComposite &mode, const PinConfigComposite &pinConfig)
	{
		int pinMap[16] = {
			-1, -1, 
			-1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1,
			-1, -1, -1, -1
		};
		firstPixelOffset = mode.hSync + mode.hBack;
		colorClock0x1000Periods = 0x1000L*(float)mode.pixelClock/mode.colorClock; // pixels per 0x1000 color cycles
		bufferVDiv = mode.vDiv;
		bufferInterlaced = mode.interlaced;
		bufferPhaseAlternating = mode.phaseAlternating;
		return initDAC(mode, pinMap, 16, -1);
	}

	bool initDAC(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin)
	{
		this->mode = mode;
		int xres = mode.hRes;
		int yres = mode.vRes / mode.vDiv;
		propagateResolution(xres, yres);
		totalLines = mode.linesPerFrame;
		allocateLineBuffers();
		currentLine = 0;
		vSyncPassed = false;
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount, clockPin);
		enableDAC(outputPin==25?1:2);
		startTX();
		return true;
	}

	virtual int bytesPerSample() const
	{
		return 2;
	}

	virtual float pixelAspect() const
	{
		return 1;
	}

	virtual void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
	}

	int outputPin = 25;
	bool voltageDivider = false;

	virtual BufferUnit **allocateFrameBuffer()
	{
		return (BufferUnit **)DMABufferDescriptor::allocateDMABufferArray(yres, mode.hRes * bytesPerSample(), true, 0x01000100*levelBlanking);
	}

	virtual void allocateLineBuffers()
	{
		allocateLineBuffers((void **)frameBuffers[0]);
	}

	//void *hSyncLineBuffer[2];

	void *vBlankLineBuffer[2];

	//Vertical sync
	//4 possible Half Lines (HL):
	// NormalFront (NF), NormalBack (NB), Equalizing (EQ), Sync (SY)
	void *normalFrontLineBuffer;
	void *equalizingLineBuffer;
	void *vSyncLineBuffer;
	void *normalBackLineBuffer;

	//complete ring of buffer descriptors for one frame
	virtual void allocateLineBuffers(void **frameBuffer)
	{
		//lenght of each line
		int samples = mode.hFront + mode.hSync + mode.hBack + mode.hRes;
		int bytes = samples * bytesPerSample();
		int samplesHL = samples/2;
		int bytesHL = bytes/2;
		int samplesHSync = mode.hFront + mode.hSync + mode.hBack;
		int bytesHSync = samplesHSync * bytesPerSample();

		//create and fill the buffers with their default values

		//create the buffers
		//1 blank prototype line for vFront and vBack
		vBlankLineBuffer[0] = DMABufferDescriptor::allocateBuffer(bytes, true, 0x01000100*levelBlanking);
		if(mode.phaseAlternating)
		{
			vBlankLineBuffer[1] = DMABufferDescriptor::allocateBuffer(bytes, true, 0x01000100*levelBlanking);
		} else {
			vBlankLineBuffer[1] = vBlankLineBuffer[0];
		}
		//1 prototype for each HL type in vSync
		equalizingLineBuffer = DMABufferDescriptor::allocateBuffer(bytesHL, true, 0x01000100*levelBlanking);
		vSyncLineBuffer = DMABufferDescriptor::allocateBuffer(bytesHL, true, 0x01000100*levelBlanking);
		normalFrontLineBuffer = vBlankLineBuffer[0];
		normalBackLineBuffer = (void*)&(((uint8_t*)vBlankLineBuffer[0])[bytesHL]);
		////1 prototype for hSync
		//hSyncLineBuffer[0] = DMABufferDescriptor::allocateBuffer(bytesHSync, true, 0x01000100*levelBlanking);
		//if(mode.phaseAlternating)
		//{
			//hSyncLineBuffer[1] = DMABufferDescriptor::allocateBuffer(bytesHSync, true, 0x01000100*levelBlanking);
		//} else {
			//hSyncLineBuffer[1] = hSyncLineBuffer[0];
		//}
		//n lines as buffer for active lines
		//already allocated in allocateFrameBuffer

		//fill the buffers with their default values
		//(bytesPerSample() == 2)(actually only MSByte is used)
		for (int i = 0; i < samples; i++)
		{
			//hsync signal
			if (i >= mode.hFront && i < (mode.hFront + mode.hSync))
			{
				//blank line
				((unsigned short *)vBlankLineBuffer[0])[i ^ 1] = levelSync << 8;
				if(mode.phaseAlternating)
					((unsigned short *)vBlankLineBuffer[1])[i ^ 1] = levelSync << 8;
				////hsync
				//((unsigned short *)hSyncLineBuffer[0])[i ^ 1] = levelSync << 8;
				//if(mode.phaseAlternating)
					//((unsigned short *)hSyncLineBuffer[1])[i ^ 1] = levelSync << 8;
			}
			//color burst // pixel counting starts at the hsync pulse beginning
			if ( mode.colorClock > 0 &&
			     i >= (mode.hFront + mode.hSync + mode.burstStart) &&
			     i < (mode.hFront + mode.hSync + mode.burstStart + mode.burstLength)
			   )
			{
				if(mode.phaseAlternating==false)
				{
					//blank line
					((unsigned short *)vBlankLineBuffer[0])[i ^ 1] =
					   (unsigned short)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) + PI)*amplitudeBurst
									   ) << 8;
					////hsync
					//((unsigned short *)hSyncLineBuffer[0])[i ^ 1] =
					   //(unsigned short)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) + PI)*amplitudeBurst
									   //) << 8;
				} else {
					//blank line
					((unsigned short *)vBlankLineBuffer[0])[i ^ 1] =
					   (unsigned short)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) + PI*3/4)*amplitudeBurst
									   ) << 8;
					((unsigned short *)vBlankLineBuffer[1])[i ^ 1] =
					   (unsigned short)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) - PI*3/4)*amplitudeBurst
									   ) << 8;
					////hsync
					//((unsigned short *)hSyncLineBuffer[0])[i ^ 1] =
					   //(unsigned short)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) + PI*3/4 - PI*3/4)*amplitudeBurst
									   //) << 8;
					//((unsigned short *)hSyncLineBuffer[1])[i ^ 1] =
					   //(unsigned short)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) - PI*3/4 - PI*3/4)*amplitudeBurst
									   //) << 8;
				}
			}
			//equalizing signal
			if (i >= mode.hFront && i < (mode.hFront + mode.hSync/2))
			{
				//equalizing
				((unsigned short *)equalizingLineBuffer)[i ^ 1] = levelSync << 8;
			}
			//vertical sync signal
			if (i >= mode.hFront && i < (mode.hFront + (samplesHL - mode.hSync)))
			{
				//vsync // hFront should never be bigger than hSync or this overflows
				((unsigned short *)vSyncLineBuffer)[i ^ 1] = levelSync << 8;
			}
		}


		//allocate DMA buffer descriptors for the whole frame
		dmaBufferDescriptorCount = mode.linesPerFrame * 2;
		dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(dmaBufferDescriptorCount);
		//link all buffer descriptors in a ring
		for (int i = 0; i < dmaBufferDescriptorCount; i++)
			dmaBufferDescriptors[i].next(dmaBufferDescriptors[(i + 1) % dmaBufferDescriptorCount]);

		//assign the buffers accross the DMA buffer descriptors
		//CONVENTION: the frame starts after the last non-sync line of previous frame
		int d = 0;
		//pre-line
		int consumelines = mode.vOPreRegHL;
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHL);
			dmaBufferDescriptors[d++].setBuffer(normalBackLineBuffer, bytesHL);
			consumelines-=2;
		}
		//NF
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHL);
		}
		//EQ
		consumelines = mode.vPreEqHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(equalizingLineBuffer, bytesHL);
		//SY
		consumelines = mode.vSyncHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(vSyncLineBuffer, bytesHL);
		//EQ
		consumelines = mode.vPostEqHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(equalizingLineBuffer, bytesHL);
		//NB
		consumelines = mode.vOPostRegHL;
		if((consumelines & 1) == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(normalBackLineBuffer, bytesHL);
			consumelines--;
		}
		//post-line
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHL);
			dmaBufferDescriptors[d++].setBuffer(normalBackLineBuffer, bytesHL);
			consumelines-=2;
		}

		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHL);
			dmaBufferDescriptors[d++].setBuffer(normalBackLineBuffer, bytesHL);
		}
		for (int i = 0; i < mode.vActive; i++)
		{
			dmaBufferDescriptors[d].setBuffer(vBlankLineBuffer[(d/2)&1], bytesHSync);
			d++;
			dmaBufferDescriptors[d++].setBuffer(frameBuffer[(i*(mode.interlaced?2:1) - (mode.interlaced?1:0)) / mode.vDiv], mode.hRes * bytesPerSample());
		}
		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHL);
			dmaBufferDescriptors[d++].setBuffer(normalBackLineBuffer, bytesHL);
		}

		// here d should be linesPerFrame*2 if mode is progressive
		// and linesPerFrame*2 / 2 if mode is interlaced
		if(mode.interlaced)
		{

		//pre-line
		int consumelines = mode.vEPreRegHL;
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHL);
			dmaBufferDescriptors[d++].setBuffer(normalBackLineBuffer, bytesHL);
			consumelines-=2;
		}
		//NF
		if(consumelines == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHL);
		}
		//EQ
		consumelines = mode.vPreEqHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(equalizingLineBuffer, bytesHL);
		//SY
		consumelines = mode.vSyncHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(vSyncLineBuffer, bytesHL);
		//EQ
		consumelines = mode.vPostEqHL;
		for (int i = 0; i < consumelines; i++)
			dmaBufferDescriptors[d++].setBuffer(equalizingLineBuffer, bytesHL);
		//NB
		consumelines = mode.vEPostRegHL;
		if((consumelines & 1) == 1)
		{
			dmaBufferDescriptors[d++].setBuffer(normalBackLineBuffer, bytesHL);
			consumelines--;
		}
		//post-line
		while(consumelines>=2)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHL);
			dmaBufferDescriptors[d++].setBuffer(normalBackLineBuffer, bytesHL);
			consumelines-=2;
		}

		for (int i = 0; i < mode.vBack; i++)
		{
			dmaBufferDescriptors[d].setBuffer(vBlankLineBuffer[(d/2)&1], bytesHL);
			d++;
			dmaBufferDescriptors[d++].setBuffer(normalBackLineBuffer, bytesHL);
		}
		for (int i = 0; i < mode.vActive; i++)
		{
			dmaBufferDescriptors[d].setBuffer(vBlankLineBuffer[(d/2)&1], bytesHSync);
			d++;
			dmaBufferDescriptors[d++].setBuffer(frameBuffer[(i*2) / mode.vDiv], mode.hRes * bytesPerSample());
		}
		for (int i = 0; i < mode.vFront; i++)
		{
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHL);
			dmaBufferDescriptors[d++].setBuffer(normalBackLineBuffer, bytesHL);
		}

		}
	}

	virtual void show(bool vSync = false)
	{
		if (!frameBufferCount)
			return;
		if (vSync)
		{
			//TODO read the I2S docs to find out
		}
		Graphics::show(vSync);
	}

  protected:
	virtual void interrupt()
	{
	}
};
