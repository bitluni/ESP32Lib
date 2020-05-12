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
	
	   55 shades                  179 shades
	
	ESP32        TV           ESP32                       TV     
	-----+                     -----+    ____ 100 ohm
	    G|-                        G|---|____|+          
	pin25|--------- Comp       pin25|---|____|+--------- Comp    
	pin26|-                    pin26|-        220 ohm
	     |                          |
	     |                          |
	-----+                     -----+                              
	
	Connect pin 25 or 26
*/
#pragma once
#include "Composite.h"
#include "../Graphics/GraphicsX6S2W8RangedSwapped.h"

#include "driver/dac.h"

#include <soc/rtc.h>
#include <driver/rtc_io.h>

class CompositeGrayDAC : public Composite, public GraphicsX6S2W8RangedSwapped
{
  public:
	CompositeGrayDAC() //DAC based modes only work with I2S0
		: Composite(0)
	{
		lineBufferCount = 3;
		colorMinValue = 23;
		syncLevel = 0;
		colorMaxValue = 77;
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
			colorMinValue = 77;
			syncLevel = 0;
			colorMaxValue = 255;
		}
		//values must be shifted to the MSByte to be output
		//which is equivalent to multiplying by 256
		//instead of shifting, do not divide here:
		//colorDepthConversionFactor = (colorMaxValue - colorMinValue + 1)/256;
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;
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
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;
		return initDAC(mode, pinMap, 16, -1);
	}

	bool initDAC(const ModeComposite &mode, const int *pinMap, const int bitCount, const int clockPin)
	{
		i2s_dev_t *i2sDevices[] = {&I2S0, &I2S1};
		this->mode = mode;
		int xres = mode.hRes;
		int yres = mode.vRes / mode.vDiv;
		propagateResolution(xres, yres);
		totalLines = mode.linesPerFrame;
		allocateLineBuffers();
		currentLine = 0;
		vSyncPassed = false;
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount, clockPin);
		volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
		i2s.conf2.lcd_en = 1;
		i2s.conf.tx_right_first = 1;
		i2s.conf2.camera_en = 0;
		dac_i2s_enable();
		dac_output_enable(outputPin==25?DAC_CHANNEL_1:DAC_CHANNEL_2);
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

	virtual InternalColor **allocateFrameBuffer()
	{
		return (InternalColor **)DMABufferDescriptor::allocateDMABufferArray(yres, mode.hRes * bytesPerSample(), true, 0x01000100*colorMinValue);
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
		vBlankLineBuffer = DMABufferDescriptor::allocateBuffer(bytes, true, 0x01000100*colorMinValue);
		//1 prototype for each HL type in vSync
		equalizingLineBuffer = DMABufferDescriptor::allocateBuffer(bytesHL, true, 0x01000100*colorMinValue);
		vSyncLineBuffer = DMABufferDescriptor::allocateBuffer(bytesHL, true, 0x01000100*colorMinValue);
		normalFrontLineBuffer = vBlankLineBuffer;
		normalBackLineBuffer = (void*)&(((uint8_t*)vBlankLineBuffer)[bytesHL]);
		//1 prototype for hSync
		hSyncLineBuffer = DMABufferDescriptor::allocateBuffer(bytesHSync, true, 0x01000100*colorMinValue);
		//n lines as buffer for active lines
		//already allocated in allocateFrameBuffer

		//fill the buffers with their default values
		//(bytesPerSample() == 2)(actually only MSByte is used)
		for (int i = 0; i < samples; i++)
		{
			if (i >= mode.hFront && i < (mode.hFront + mode.hSync))
			{
				//blank line
				((unsigned short *)vBlankLineBuffer)[i ^ 1] = syncLevel << 8;
				//hsync
				((unsigned short *)hSyncLineBuffer)[i ^ 1] = syncLevel << 8;
			}
			if (i >= mode.hFront && i < (mode.hFront + mode.hSync/2))
			{
				//equalizing
				((unsigned short *)equalizingLineBuffer)[i ^ 1] = syncLevel << 8;
			}
			if (i >= mode.hFront && i < (mode.hFront + (samplesHL - mode.hSync)))
			{
				//vsync // hFront should never be bigger than hSync or this overflows
				((unsigned short *)vSyncLineBuffer)[i ^ 1] = syncLevel << 8;
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
	}

  protected:
	virtual void interrupt()
	{
	}
};
