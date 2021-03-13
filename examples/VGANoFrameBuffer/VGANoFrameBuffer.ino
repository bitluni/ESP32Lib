//This example shows how to draw directly to I2S with no frame buffer. It only allows low computation times per pixels
//to be able to serve I2S fast enough
//You need to connect a VGA screen cable and an external DAC (simple R2R does the job) to the pins specified below.
//cc by-sa 4.0 license
//bitluni

#include <ESP32Video.h>
#include <math.h>

//pin configuration
const int redPins[] = {2, 4, 12, 13, 14};
const int greenPins[] = {15, 16, 17, 18, 19};
const int bluePins[] = {21, 22, 23, 27};
const int hsyncPin = 32;
const int vsyncPin = 33;

//color palette
unsigned long rainbow[256];
long frameNumber = 0;

//Our own VGA Device
class MyVGA : public VGA14BitI
{
	public:
	//override this routine that is called during init to link the custom interrupt
	void propagateResolution(const int xres, const int yres)
	{
		setResolution(xres, yres);
		interruptStaticChild = &MyVGA::custominterrupt;
	}
	protected:
	//override frame buffer allocation
	virtual Color **allocateFrameBuffer()
	{
		return 0;
	}
	static void custominterrupt(void *arg);
	static void custominterruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits, void *arg);
};

//custominterrupt just copied from VGA14Bit except the initial and end lines
void IRAM_ATTR MyVGA::custominterrupt(void *arg)
{
	MyVGA * staticthis = (MyVGA *)arg;
//Modified for MyVGA until here

	//fix for skipped lines due to skipped interupts during wifi activity
	DMABufferDescriptor *currentDmaBufferDescriptor = (DMABufferDescriptor *)REG_READ(I2S_OUT_EOF_DES_ADDR_REG(staticthis->i2sIndex));
	staticthis->dmaBufferDescriptorActive = ((uint32_t)currentDmaBufferDescriptor - (uint32_t)staticthis->dmaBufferDescriptors)/sizeof(DMABufferDescriptor);
	staticthis->currentLine = staticthis->dmaBufferDescriptorActive; //equivalent in this configuration

	int vInactiveLinesCount = staticthis->mode.vFront + staticthis->mode.vSync + staticthis->mode.vBack;

	//render ahead (the lenght of buffered lines)
	int renderLine = (staticthis->currentLine + staticthis->lineBufferCount) % staticthis->totalLines;

	if (renderLine >= vInactiveLinesCount)
	{
		int renderActiveLine = renderLine - vInactiveLinesCount;
		unsigned long *pixels = &((unsigned long *)staticthis->vActiveLineBuffer[renderActiveLine % staticthis->lineBufferCount])[(staticthis->mode.hSync + staticthis->mode.hBack) / 2];
		unsigned long base = (staticthis->hsyncBitI | staticthis->vsyncBitI) * 0x10001;

		int y = renderActiveLine / staticthis->mode.vDiv;
		if (y >= 0 && y < staticthis->mode.vRes)
//Modified for MyVGA from here
		staticthis->custominterruptPixelLine(y, pixels, base, arg);
	}

	if (renderLine == 0)
	{
		staticthis->vSyncPassed = true;
		//added to count the frames
		frameNumber++;
	}
}

//Custom rendering routine to draw each line
void IRAM_ATTR MyVGA::custominterruptPixelLine(int y, unsigned long *pixels, unsigned long syncBits, void *arg)
{
	MyVGA * staticthis = (MyVGA *)arg;
	for (int x = 0; x < staticthis->mode.hRes / 2; x++)
	{
		//writing two pixels improves speed drastically (avoids memory reads)
		pixels[x] = syncBits | rainbow[(x - y + frameNumber) & 255];
	}
}

//get an instance
MyVGA videodisplay;

//initial setup
void setup()
{
	//color palette calculations
	const float cb[][3] = {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {1, 0, 1}};
	for (int i = 0; i < 256; i++)
	{
		//interpolate the colors from the cb array and calculate the R5G5B5 color
		float s = 6.f / 256 * i;
		int n = int(s);
		float f = s - n;
		float fi = (1 - f);
		const float *cf = cb[n];
		const float *cfn = cb[(n + 1) % 6];
		int r = int((fi * cf[0] * 0b11111) + (f * cfn[0] * 0b11111));
		int g = int((fi * cf[1] * 0b11111) + (f * cfn[1] * 0b11111));
		int b = int((fi * cf[2] * 0b1111) + (f * cfn[2] * 0b1111));
		int c = r | (g << 5) | (b << 10);
		rainbow[i] = c;
	}
	//prepare writing two pixels at once (we shift the palette by a pixel)
	for (int i = 0; i < 256; i++)
		rainbow[i] |= rainbow[(i - 1) & 255] << 16;

	//initializing the vga with a high mode where a frambuffer would never fit into the memory
	videodisplay.init(VGAMode::MODE500x480, redPins, greenPins, bluePins, hsyncPin, vsyncPin);
}

//idle, everything is happening in the interrupt
void loop()
{
	delay(10);
}
