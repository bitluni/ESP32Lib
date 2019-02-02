#include "I2SVGA.h"
#include <soc/frc_timer_reg.h>

//maximum pixel clock with apll is 36249999.
//hfront hsync hback pixels vfront vsync vback lines divx divy pixelclock
const int I2SVGA::MODE320x480[] = {16, 96, 52, 640, 11, 2, 31, 480, 2, 1, 25175000};
const int I2SVGA::MODE320x240[] = {16, 96, 52, 640, 11, 2, 31, 480, 2, 2, 25175000};
const int I2SVGA::MODE320x120[] = {16, 96, 52, 640, 11, 2, 31, 480, 2, 4, 25175000};
const int I2SVGA::MODE320x400[] = {16, 96, 48, 640, 11, 2, 31, 400, 2, 1, 25175000};
const int I2SVGA::MODE320x200[] = {16, 96, 48, 640, 11, 2, 31, 400, 2, 2, 25175000};
const int I2SVGA::MODE320x100[] = {16, 96, 48, 640, 11, 2, 31, 400, 2, 4, 25175000};
const int I2SVGA::MODE360x400[] = {16, 108, 56, 720, 11, 2, 32, 400, 2, 1, 28322000};
const int I2SVGA::MODE360x200[] = {16, 108, 56, 720, 11, 2, 32, 400, 2, 2, 28322000};
const int I2SVGA::MODE360x100[] = {16, 108, 56, 720, 11, 2, 32, 400, 2, 4, 28322000};
const int I2SVGA::MODE360x350[] = {16, 108, 56, 720, 11, 2, 32, 350, 2, 1, 28322000};
const int I2SVGA::MODE360x175[] = {16, 108, 56, 720, 11, 2, 32, 350, 2, 2, 28322000};
const int I2SVGA::MODE360x88[] = {16, 108, 56, 720, 11, 2, 31, 350, 2, 4, 28322000};

//not supported on any of my screens
const int I2SVGA::MODE384x576[] = {24, 80, 104, 768, 1, 3, 17, 576, 2, 1, 34960000};
const int I2SVGA::MODE384x288[] = {24, 80, 104, 768, 1, 3, 17, 576, 2, 2, 34960000};
const int I2SVGA::MODE384x144[] = {24, 80, 104, 768, 1, 3, 17, 576, 2, 4, 34960000};
const int I2SVGA::MODE384x96[] = {24, 80, 104, 768, 1, 3, 17, 576, 2, 6, 34960000};

//not stable (can't reach 40MHz pixel clock, it's clipped by the driver to 36249999 at undivided resolution)
//you can mod the timings a bit the get it running on your system
const int I2SVGA::MODE400x300[] = {40, 128, 88, 800, 1, 4, 23, 600, 2, 2, 39700000};
const int I2SVGA::MODE400x150[] = {40, 128, 88, 800, 1, 4, 23, 600, 2, 4, 39700000};
const int I2SVGA::MODE400x100[] = {40, 128, 88, 800, 1, 4, 23, 600, 2, 6, 39700000};
const int I2SVGA::MODE200x150[] = {40, 128, 88, 800, 1, 4, 23, 600, 4, 4, 39700000};

//you took your time to look at the code. try this mode.. 460 pixels horizontal it's based on 640x480
const int I2SVGA::HIDDEN_MODE0[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 1, 36249999};
const int I2SVGA::HIDDEN_MODE1[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 2, 36249999};
const int I2SVGA::HIDDEN_MODE2[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 4, 36249999};
const int I2SVGA::HIDDEN_MODE3[] = {24, 136, 76, 920, 11, 2, 31, 480, 2, 5, 36249999};

I2SVGA::I2SVGA(const int i2sIndex)
	: I2S(i2sIndex), Graphics<unsigned short>()
{
}

bool I2SVGA::init(const int *mode, const int *redPins, const int *greenPins, const int *bluePins, const int hsyncPin, const int vsyncPin, int lineBufferCount)
{
	initBuffers(mode[3] / mode[8], mode[7] / mode[9], 1337);
	this->vsyncPin = vsyncPin;
	this->hsyncPin = hsyncPin;
	hdivider = mode[8];
	vdivider = mode[9];
	hfront = mode[0] / hdivider;
	hsync = mode[1] / hdivider;
	hback = mode[2] / hdivider;
	totalLines = mode[4] + mode[5] + mode[6] + mode[7];
	vfront = mode[4];
	vsync = mode[5];
	vback = mode[6];

	rgbMask = 0x3fff;
	hsyncBit = 0x0000;
	vsyncBit = 0x0000;
	hsyncBitI = 0x4000;
	vsyncBitI = 0x8000;

	int pinMap[24];
	for (int i = 0; i < 8; i++)
		pinMap[i] = -1;
	for (int i = 0; i < 5; i++)
	{
		pinMap[i + 8] = redPins[i];
		pinMap[i + 13] = greenPins[i];
		if (i < 4)
			pinMap[i + 18] = bluePins[i];
	}
	pinMap[22] = hsyncPin;
	pinMap[23] = vsyncPin;
	initParallelOutputMode(pinMap, mode[10] / hdivider);
	allocateDMABuffersVGA(lineBufferCount);
	currentLine = 0;
	startTX();
	return true;
}

/// simple ringbuffer of blocks of size bytes each
void I2SVGA::allocateDMABuffersVGA(const int lines)
{
	const int bytesPerPixel = 2;
	dmaBufferCount = lines;
	dmaBuffers = (DMABuffer **)malloc(sizeof(DMABuffer *) * dmaBufferCount);
	if (!dmaBuffers)
		DEBUG_PRINTLN("Failed to allocate DMABuffer array");
	for (int i = 0; i < dmaBufferCount; i++)
	{
		dmaBuffers[i] = DMABuffer::allocate((hfront + hsync + hback + xres) * bytesPerPixel); //front porch + hsync + back porch + pixels
		if (i)
			dmaBuffers[i - 1]->next(dmaBuffers[i]);
	}
	dmaBuffers[dmaBufferCount - 1]->next(dmaBuffers[0]);
}

void I2SVGA::interrupt()
{
	unsigned long *signal = (unsigned long *)dmaBuffers[dmaBufferActive]->buffer;
	unsigned long *pixels = &((unsigned long *)dmaBuffers[dmaBufferActive]->buffer)[(hfront + hsync + hback) / 2];
	unsigned long base, baseh;
	if (currentLine >= vfront && currentLine < vfront + vsync)
	{
		baseh = (vsyncBit | hsyncBit) | ((vsyncBit | hsyncBit) << 16);
		base = (vsyncBit | hsyncBitI) | ((vsyncBit | hsyncBitI) << 16);
	}
	else
	{
		baseh = (vsyncBitI | hsyncBit) | ((vsyncBitI | hsyncBit) << 16);
		base = (vsyncBitI | hsyncBitI) | ((vsyncBitI | hsyncBitI) << 16);
	}
	for (int i = 0; i < hfront / 2; i++)
		signal[i] = base;
	for (int i = hfront / 2; i < (hfront + hsync) / 2; i++)
		signal[i] = baseh;
	for (int i = (hfront + hsync) / 2; i < (hfront + hsync + hback) / 2; i++)
		signal[i] = base;

	int y = (currentLine - vfront - vsync - vback) / vdivider;
	if (y >= 0 && y < yres)
	{
		unsigned short *line = frame[y];
		for (int i = 0; i < xres / 2; i++)
		{
			//writing two pixels improves speed drastically (avoids reading in higher word)
			pixels[i] = base | (line[i * 2 + 1] & rgbMask) | ((line[i * 2] & rgbMask) << 16);
		}
	}
	else
		for (int i = 0; i < xres / 2; i++)
		{
			pixels[i] = base | (base << 16);
		}
	currentLine = (currentLine + 1) % totalLines;
	dmaBufferActive = (dmaBufferActive + 1) % dmaBufferCount;
}

float I2SVGA::pixelAspect() const
{
	return float(vdivider) / hdivider;
}




/* code dumpster
	/*uint32_t start, finish, current;
  start = REG_READ(FRC_TIMER_COUNT_REG(1));
  gpio_set_level((gpio_num_t)hsyncPin, 0);
  finish = start + 8 * 38;
  do
  {
    current = REG_READ(FRC_TIMER_COUNT_REG(1));
  }
  while(current > start && current < finish);
  //hsync done
  //gpio_set_level((gpio_num_t)hsyncPin, 1);
*/