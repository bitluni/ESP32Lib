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

#ifdef ESP32

#include "VGAI2SOverlapping.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/BufferLayouts/BLpx6sz8swmx2yshmxy.h"


//#include "esp_heap_caps.h"
//#include "soc/soc.h"
#include "soc/gpio_sig_map.h"
//#include "soc/i2s_reg.h"
//#include "soc/i2s_struct.h"
//#include "soc/io_mux_reg.h"
#include "driver/gpio.h"


class VGA6MonochromeVGAMadnessMultimonitor : public VGAI2SOverlapping< BLpx1sz8sw2sh0, Graphics<ColorW1X7, BLpx6sz8swmx2yshmxy, CTBIdentity> >
{
  public:
	VGA6MonochromeVGAMadnessMultimonitor()
		: VGAI2SOverlapping< BLpx1sz8sw2sh0, Graphics<ColorW1X7, BLpx6sz8swmx2yshmxy, CTBIdentity> >(1)
	{
		frontColor = 0x1;
	}

	bool init(const Mode &mode,
			  const int R0Pin, const int G0Pin, const int B0Pin,
			  const int R1Pin, const int G1Pin, const int B1Pin,
			  const int R2Pin, const int G2Pin, const int B2Pin,
			  const int R3Pin, const int G3Pin, const int B3Pin,
			  const int R4Pin, const int G4Pin, const int B4Pin,
			  const int R5Pin, const int G5Pin, const int B5Pin,
			  const int hsyncPin, const int vsyncPin, const int clockPin = -1,
			  const int horMonitorCount = 3, const int verMonitorCount = 2)
	{
		const int bitCount = 8;
		int pinMap[bitCount] = {
			R0Pin, R1Pin,
			R2Pin, R3Pin,
			R4Pin, R5Pin,
			hsyncPin, vsyncPin
		};
		mx = horMonitorCount;
		my = verMonitorCount;
		pinOutputMap[0] = R0Pin; pinOutputMap[1] = R1Pin;
		pinOutputMap[2] = R2Pin; pinOutputMap[3] = R3Pin;
		pinOutputMap[4] = R4Pin; pinOutputMap[5] = R5Pin;
		pinOutputMap[6] = G0Pin; pinOutputMap[7] = G1Pin;
		pinOutputMap[8] = G2Pin; pinOutputMap[9] = G3Pin;
		pinOutputMap[10] = G4Pin; pinOutputMap[11] = G5Pin;
		pinOutputMap[12] = B0Pin; pinOutputMap[13] = B1Pin;
		pinOutputMap[14] = B2Pin; pinOutputMap[15] = B3Pin;
		pinOutputMap[16] = B4Pin; pinOutputMap[17] = B5Pin;

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

	virtual bool initengine(const Mode &mode, const int *pinMap, const int bitCount, const int clockPin = -1, int descriptorsPerLine = 2)
	{
		const int deviceBaseIndex[] = {I2S0I_DATA_IN0_IDX, I2S1I_DATA_IN0_IDX};
		initenginePreparation(mode, pinMap, bitCount, clockPin, descriptorsPerLine);
		initParallelOutputMode(pinMap, mode.pixelClock, bitCount, clockPin);
		for (int i = 0; i < 18; i++)
			if (pinOutputMap[i] > -1)
			{
				PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[pinOutputMap[i]], PIN_FUNC_GPIO);
				gpio_set_direction((gpio_num_t)pinOutputMap[i], (gpio_mode_t)GPIO_MODE_DEF_OUTPUT);
				//rtc_gpio_set_drive_capability((gpio_num_t)pinMap[i], (gpio_drive_cap_t)GPIO_DRIVE_CAP_3 );
				//signal_idx == 0x100, cancel output put to the gpio
				gpio_matrix_out(pinOutputMap[i], 0x100, false, false);
				if ((frontGlobalColor[i % 6]>>(i/6))&1) gpio_matrix_out(pinOutputMap[i], deviceBaseIndex[1] + (i % 6), false, false);
				//modify background only if it is compatible with front color:
				//components of... (background MUST be in foreground) AND (foreground MUST SURPASS background)
				if ( ((backGlobalColor[i % 6] & frontGlobalColor[i % 6]) == backGlobalColor[i % 6]) && ( (frontGlobalColor[i % 6] & ((backGlobalColor[i % 6] & frontGlobalColor[i % 6])^0b00000111))>0) )
					if ((backGlobalColor[i % 6]>>(i/6))&1) gpio_matrix_out(pinOutputMap[i], deviceBaseIndex[1] + (8*bytesPerBufferUnit()-2), (mode.hSyncPolarity==1)?false:true, false);
			}
		startTX();
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

	int pinOutputMap[18];

	//This is interpreted as 3-bit color:
	ColorR1G1B1A1X4::Color frontGlobalColor[6] = {0xf,0xf,0xf,0xf,0xf,0xf};
	ColorR1G1B1A1X4::Color backGlobalColor[6] = {0x0,0x0,0x0,0x0,0x0,0x0};

	int selectedMonitor = -1;

	void setMonitor(int monitor = -1)
	{
		if (monitor > 5) return;
		if (monitor < -1) return;
		selectedMonitor = monitor;
	}

	void setFrontGlobalColor(int r, int g, int b, int a = 255)
	{
		if(selectedMonitor < 0)
		{
			for(int i=0;i<6;i++) frontGlobalColor[i] = ColorR1G1B1A1X4::static_RGBA(r, g, b, a);
		} else {
			frontGlobalColor[selectedMonitor] = ColorR1G1B1A1X4::static_RGBA(r, g, b, a);
		}
	}

	void setBackGlobalColor(int r, int g, int b, int a = 255)
	{
		if(selectedMonitor < 0)
		{
			for(int i=0;i<6;i++) backGlobalColor[i] = ColorR1G1B1A1X4::static_RGBA(r, g, b, a);
		} else {
			backGlobalColor[selectedMonitor] = ColorR1G1B1A1X4::static_RGBA(r, g, b, a);
		}
	}

	void clear(Color color = 0)
	override
	{
		BufferGraphicsUnit bufferUnaffectedBits = (backBuffer[0][0])&( vsyncBit | hsyncBit | vsyncBitI | hsyncBitI );
		BufferGraphicsUnit newColor = (BufferGraphicsUnit)( graphics_coltobuf(color & static_colormask(), 0, 0)*((0xffff) & (~( vsyncBit | hsyncBit | vsyncBitI | hsyncBitI ))) | bufferUnaffectedBits );
		for (int y = 0; y < this->wy; y++)
			for (int x = 0; x < this->wx; x++)
				backBuffer[y][x] = newColor;
	}

};

#endif
