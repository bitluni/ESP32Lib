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
	
	   55 shades                  255 shades
	
	ESP32        VGA           ESP32                       VGA     
	-----+                     -----+    ____ 100 ohm              
	    G|-   +---- R              G|---|____|+         +---- R    
	pin25|----+---- G          pin25|---|____|+---------+---- G    
	pin26|-   +---- B          pin26|-        220 ohm   +---- B    
	pin X|--------- HSYNC      pin X|------------------------ HSYNC
	pin Y|--------- VSYNC      pin Y|------------------------ VSYNC
	-----+                     -----+                              
	
	Connect pin 25 or 26
	Connect the 3 channels in parallel or whatever combination of them
	  depending on the monochrome color of choice
*/
#pragma once
#include "VGA.h"
#include "../Graphics/GraphicsX6S2W8RangedSwapped.h"

#include "driver/dac.h"

#include <soc/rtc.h>
#include <driver/rtc_io.h>



class VGA8BitDAC : public VGA, public GraphicsX6S2W8RangedSwapped
{
  public:
	VGA8BitDAC() //DAC based modes only work with I2S0
		: VGA(0)
	{
		lineBufferCount = 3;
		colorMaxValue = 54;
	}

	bool init(const Mode &mode, const int hsyncPin, const int vsyncPin, const int outputPin = 25, const bool voltageDivider = false)
	{
		int pinMap[16] = {
			-1, -1, -1, -1,
			-1, -1, hsyncPin, vsyncPin,
			-1, -1, -1, -1,
			-1, -1, -1, -1
		};
		this->outputPin = outputPin;
		this->voltageDivider = voltageDivider;
		if(voltageDivider)
		{
			colorMaxValue = 255;
		}
		//values must be shifted to the MSByte to be output
		//which is equivalent to multiplying by 256
		//instead of shifting, do not divide here:
		//colorDepthConversionFactor = (colorMaxValue - colorMinValue + 1)/256;
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;
		return initDAC(mode, pinMap, 16, -1);
	}

	bool init(const Mode &mode, const PinConfig &pinConfig)
	{
		int pins[16];
		pinConfig.fill14Bit(pins);
		this->hsyncPin = pins[14];
		this->vsyncPin = pins[15];
		for (int i = 0; i < 16; i++)
		{
			pins[i] = -1;
		}
		pins[6] = this->hsyncPin;
		pins[7] = this->vsyncPin;
		colorDepthConversionFactor = colorMaxValue - colorMinValue + 1;
		return initDAC(mode, pins, 16, pinConfig.clock);
	}

	bool initDAC(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin)
	{
		i2s_dev_t *i2sDevices[] = {&I2S0, &I2S1};
		this->mode = mode;
		int xres = mode.hRes;
		int yres = mode.vRes / mode.vDiv;
		initSyncBits();
		propagateResolution(xres, yres);
		this->hsyncPin = hsyncPin;
		this->vsyncPin = vsyncPin;
		totalLines = mode.linesPerField();
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

	virtual void initSyncBits()
	{
		hsyncBitI = mode.hSyncPolarity ? 0x0040 : 0;
		vsyncBitI = mode.vSyncPolarity ? 0x0080 : 0;
		hsyncBit = hsyncBitI ^ 0x0040;
		vsyncBit = vsyncBitI ^ 0x0080;
		SBits = hsyncBitI | vsyncBitI;
	}

	virtual long syncBits(bool hSync, bool vSync)
	{
		return ((hSync ? hsyncBit : hsyncBitI) | (vSync ? vsyncBit : vsyncBitI)) * 0x00010001;
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

	virtual BufferUnit** allocateFrameBuffer()
	{
		return (BufferUnit**)DMABufferDescriptor::allocateDMABufferArray(yres, mode.hRes * bytesPerSample(), true, syncBits(false, false));
	}

	virtual void allocateLineBuffers()
	{
		VGA::allocateLineBuffers((void **)frameBuffers[0]);
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
		if(dmaBufferDescriptors)
			for (int i = 0; i < yres * mode.vDiv; i++)
				dmaBufferDescriptors[(mode.vFront + mode.vSync + mode.vBack + i) * 2 + 1].setBuffer(frontBuffer[i / mode.vDiv], mode.hRes * bytesPerSample());
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
