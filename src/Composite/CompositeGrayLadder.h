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
	
	A) R–2R resistor ladder; B) unequal rungs ladder
	
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
	Custom ladders can be used by tweaking colorMinValue and colorMaxValue
*/
#pragma once
#include "Composite.h"
#include "../Graphics/GraphicsW8RangedSwapped.h"

class CompositeGrayLadder : public Composite, public GraphicsW8RangedSwapped
{
  public:
	CompositeGrayLadder() //8 bit based modes only work with I2S1
		: Composite(1)
	{
		colorMinValue = 76;
		syncLevel = 0;
		colorMaxValue = 255;
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

		//values must be divided to fit 8bits
		//instead of using a float, bitshift 8 bits to the right later:
		//colorDepthConversionFactor = (colorMaxValue - colorMinValue + 1)/256;
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;
		return Composite::init(mode, pinMap, 8);
	}

	bool init(const ModeComposite &mode, const int *compositePins)
	{
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;
		return Composite::init(mode, compositePins, 8);
	}

	bool init(const ModeComposite &mode, const PinConfigComposite &pinConfig)
	{
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;
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

	virtual InternalColor **allocateFrameBuffer()
	{
		return (InternalColor **)DMABufferDescriptor::allocateDMABufferArray(yres, mode.hRes * bytesPerSample(), true, 0x01010101*colorMinValue);
	}

	virtual void allocateLineBuffers()
	{
		allocateLineBuffers((void **)frameBuffers[0]);
	}

	void *hSyncLineBuffer;

	void *vBlankLineBuffer;

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
		vBlankLineBuffer = DMABufferDescriptor::allocateBuffer(bytes, true, 0x01010101*colorMinValue);
		//1 prototype for each HL type in vSync
		equalizingLineBuffer = DMABufferDescriptor::allocateBuffer(bytesHL, true, 0x01010101*colorMinValue);
		vSyncLineBuffer = DMABufferDescriptor::allocateBuffer(bytesHL, true, 0x01010101*colorMinValue);
		normalFrontLineBuffer = vBlankLineBuffer;
		normalBackLineBuffer = (void*)&(((uint8_t*)vBlankLineBuffer)[bytesHL]);
		//1 prototype for hSync
		hSyncLineBuffer = DMABufferDescriptor::allocateBuffer(bytesHSync, true, 0x01010101*colorMinValue);
		//n lines as buffer for active lines
		//already allocated in allocateFrameBuffer

		//fill the buffers with their default values
		//(bytesPerSample() == 2)(actually only MSByte is used)
		for (int i = 0; i < samples; i++)
		{
			if (i >= mode.hFront && i < (mode.hFront + mode.hSync))
			{
				//blank line
				((unsigned char *)vBlankLineBuffer)[i ^ 2] = syncLevel;
				//hsync
				((unsigned char *)hSyncLineBuffer)[i ^ 2] = syncLevel;
			}
			if (i >= mode.hFront && i < (mode.hFront + mode.hSync/2))
			{
				//equalizing
				((unsigned char *)equalizingLineBuffer)[i ^ 2] = syncLevel;
			}
			if (i >= mode.hFront && i < (mode.hFront + (samplesHL - mode.hSync)))
			{
				//vsync // hFront should never be bigger than hSync or this overflows
				((unsigned char *)vSyncLineBuffer)[i ^ 2] = syncLevel;
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
		if(consumelines & 1 == 1)
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
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHSync);
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
		if(consumelines & 1 == 1)
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
			dmaBufferDescriptors[d++].setBuffer(normalFrontLineBuffer, bytesHSync);
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
		//if(dmaBufferDescriptors)
		//for (int i = 0; i < yres * mode.vDiv; i++)
			//dmaBufferDescriptors[(mode.vFront + mode.vSync + mode.vBack + i) * 2 + 1].setBuffer(frontBuffer[i / mode.vDiv], mode.hRes * bytesPerSample());
	}

	virtual void scroll(int dy, Color color)
	{
		Graphics::scroll(dy, color);
		if (frameBufferCount == 1)
			show();
	}

  protected:
	virtual void interrupt()
	{
	}
};
