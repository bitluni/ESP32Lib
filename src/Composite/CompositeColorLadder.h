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
	Custom ladders can be used by tweaking output levels member variables
*/
#pragma once
#include "Composite.h"
#include "../Graphics/GraphicsCA8Swapped.h"

class CompositeColorLadder : public Composite, public GraphicsCA8Swapped
{
  public:
	CompositeColorLadder() //8 bit based modes only work with I2S1
		: Composite(1)
	{
		lineBufferCount = 3;
		//Raw DAC output values
		//---------------------
		//levelHighClipping = 95;
		//levelWhite = 77;
		//amplitudeBurst = 11;
		//levelBlack = 23;
		//levelBlanking = 23;
		//levelLowClipping = 5;
		//levelSync = 0;
		//Normalized (with voltage divider) DAC output values
		//---------------------
		levelHighClipping = 255;
		levelWhite = 207;
		amplitudeBurst = 31;
		levelBlack = 62;
		levelBlanking = 62;
		levelLowClipping = 14;
		levelSync = 0;
	}

	bool init(const ModeComposite &mode, 
			  const int C0Pin, const int C1Pin,
			  const int C2Pin, const int C3Pin,
			  const int C4Pin, const int C5Pin,
			  const int C6Pin, const int C7Pin)
	{
		int pinMap[8] = {
			C0Pin, C1Pin,
			C2Pin, C3Pin,
			C4Pin, C5Pin,
			C6Pin, C7Pin
		};

		firstPixelOffset = mode.hSync + mode.hBack;
		colorClock0x1000Periods = 0x1000L*(float)mode.pixelClock/mode.colorClock; // pixels per 0x1000 color cycles
		bufferVDiv = mode.vDiv;
		bufferInterlaced = mode.interlaced;
		bufferPhaseAlternating = mode.phaseAlternating;
		return Composite::init(mode, pinMap, 8);
	}

	bool init(const ModeComposite &mode, const int *compositePins)
	{
		firstPixelOffset = mode.hSync + mode.hBack;
		colorClock0x1000Periods = 0x1000L*(float)mode.pixelClock/mode.colorClock; // pixels per 0x1000 color cycles
		bufferVDiv = mode.vDiv;
		bufferInterlaced = mode.interlaced;
		bufferPhaseAlternating = mode.phaseAlternating;
		return Composite::init(mode, compositePins, 8);
	}

	bool init(const ModeComposite &mode, const PinConfigComposite &pinConfig)
	{
		firstPixelOffset = mode.hSync + mode.hBack;
		colorClock0x1000Periods = 0x1000L*(float)mode.pixelClock/mode.colorClock; // pixels per 0x1000 color cycles
		bufferVDiv = mode.vDiv;
		bufferInterlaced = mode.interlaced;
		bufferPhaseAlternating = mode.phaseAlternating;
		int pins[8];
		pinConfig.fill(pins);
		return Composite::init(mode, pins, 8);
	}

	virtual int bytesPerSample() const
	{
		return 1;
	}

	virtual float pixelAspect() const
	{
		return 1;
	}

	virtual void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
	}

	virtual BufferUnit **allocateFrameBuffer()
	{
		return (BufferUnit **)DMABufferDescriptor::allocateDMABufferArray(yres, mode.hRes * bytesPerSample(), true, 0x01010101*levelBlanking);
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
		vBlankLineBuffer[0] = DMABufferDescriptor::allocateBuffer(bytes, true, 0x01010101*levelBlanking);
		if(mode.phaseAlternating)
		{
			vBlankLineBuffer[1] = DMABufferDescriptor::allocateBuffer(bytes, true, 0x01010101*levelBlanking);
		} else {
			vBlankLineBuffer[1] = vBlankLineBuffer[0];
		}
		//1 prototype for each HL type in vSync
		equalizingLineBuffer = DMABufferDescriptor::allocateBuffer(bytesHL, true, 0x01010101*levelBlanking);
		vSyncLineBuffer = DMABufferDescriptor::allocateBuffer(bytesHL, true, 0x01010101*levelBlanking);
		normalFrontLineBuffer = vBlankLineBuffer[0];
		normalBackLineBuffer = (void*)&(((uint8_t*)vBlankLineBuffer[0])[bytesHL]);
		////1 prototype for hSync
		//hSyncLineBuffer[0] = DMABufferDescriptor::allocateBuffer(bytesHSync, true, 0x01010101*levelBlanking);
		//if(mode.phaseAlternating)
		//{
			//hSyncLineBuffer[1] = DMABufferDescriptor::allocateBuffer(bytesHSync, true, 0x01010101*levelBlanking);
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
				((unsigned char *)vBlankLineBuffer[0])[i ^ 2] = levelSync;
				if(mode.phaseAlternating)
					((unsigned char *)vBlankLineBuffer[1])[i ^ 2] = levelSync;
				////hsync
				//((unsigned char *)hSyncLineBuffer[0])[i ^ 2] = levelSync;
				//if(mode.phaseAlternating)
					//((unsigned char *)hSyncLineBuffer[1])[i ^ 2] = levelSync;
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
					((unsigned char *)vBlankLineBuffer[0])[i ^ 2] =
					   (unsigned char)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) + PI)*amplitudeBurst
									   );
					////hsync
					//((unsigned char *)hSyncLineBuffer[0])[i ^ 2] =
					   //(unsigned char)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) + PI)*amplitudeBurst
									   //);
				} else {
					//blank line
					((unsigned char *)vBlankLineBuffer[0])[i ^ 2] =
					   (unsigned char)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) + PI*3/4)*amplitudeBurst
									   );
					((unsigned char *)vBlankLineBuffer[1])[i ^ 2] =
					   (unsigned char)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) - PI*3/4)*amplitudeBurst
									   );
					////hsync
					//((unsigned char *)hSyncLineBuffer[0])[i ^ 2] =
					   //(unsigned char)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) + PI*3/4 - PI*3/4)*amplitudeBurst
									   //);
					//((unsigned char *)hSyncLineBuffer[1])[i ^ 2] =
					   //(unsigned char)(levelBlanking + sin(((double)(i - mode.hFront)/((double)mode.pixelClock/(double)mode.colorClock))*(2*PI) - PI*3/4 - PI*3/4)*amplitudeBurst
									   //);
				}
			}
			//equalizing signal
			if (i >= mode.hFront && i < (mode.hFront + mode.hSync/2))
			{
				//equalizing
				((unsigned char *)equalizingLineBuffer)[i ^ 1] = levelSync;
			}
			//vertical sync signal
			if (i >= mode.hFront && i < (mode.hFront + (samplesHL - mode.hSync)))
			{
				//vsync // hFront should never be bigger than hSync or this overflows
				((unsigned char *)vSyncLineBuffer)[i ^ 1] = levelSync;
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
