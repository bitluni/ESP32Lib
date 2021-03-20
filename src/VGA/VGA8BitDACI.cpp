/*
	Author: Martin-Laclaustra 2021
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/

#include "VGA8BitDACI.h"

void IRAM_ATTR VGA8BitDACI::interrupt(void *arg)
{
	VGA8BitDACI * staticthis = (VGA8BitDACI *)arg;

	//obtain currently rendered line from the buffer just read, based on the conventioned ordering and buffers per line
	staticthis->currentLine = staticthis->dmaBufferDescriptorActive >> ( (staticthis->descriptorsPerLine==2) ? 1 : 0 );

	//in the case of two buffers per line,
	//render only when the sync half of the line ended (longer period until next interrupt)
	//else exit early
	//This might need to be revised, because it might be better to overlap and miss the second interrupt
	if ( (staticthis->descriptorsPerLine==2) && ((staticthis->dmaBufferDescriptorActive & 1) != 0) ) return;

	//TO DO: This should be precalculated outside the interrupt
	int vInactiveLinesCount = staticthis->mode.vFront + staticthis->mode.vSync + staticthis->mode.vBack;

	//render ahead (the lenght of buffered lines)
	int renderLine = (staticthis->currentLine + staticthis->lineBufferCount);
	if (renderLine >= staticthis->totalLines) renderLine -= staticthis->totalLines;

	if (renderLine >= vInactiveLinesCount)
	{
		int renderActiveLine = renderLine - vInactiveLinesCount;
		uint8_t *activeRenderingBuffer = ((uint8_t *)
		staticthis->dmaBufferDescriptors[staticthis->indexRendererDataBuffer[0] + renderActiveLine * staticthis->descriptorsPerLine + staticthis->descriptorsPerLine - 1].buffer() + staticthis->dataOffsetInLineInBytes
		);

		int y = renderActiveLine / staticthis->mode.vDiv;
		if (y >= 0 && y < staticthis->yres)
			staticthis->interruptPixelLine(y, activeRenderingBuffer, arg);
	}

	if (renderLine == 0)
		staticthis->vSyncPassed = true;
}

	//LOWER LIMIT: THE CODE BETWEEN THESE MARKS IS SHARED BETWEEN 3BIT, 6BIT, AND 14BIT

void IRAM_ATTR VGA8BitDACI::interruptPixelLine(int y, uint8_t *pixels, void *arg)
{
	VGA8BitDACI * staticthis = (VGA8BitDACI *)arg;
	unsigned long syncBits = (staticthis->hsyncBitI | staticthis->vsyncBitI) * staticthis->rendererStaticReplicate32mask;
	uint8_t *line = staticthis->frontBuffer[y];
	for (int i = 0; i < staticthis->mode.hRes / 2; i++)
	{
		//writing two pixels improves speed drastically (avoids memory reads)
		//values must be shifted to the MSByte to be output
		//which is equivalent to multiplying by 256
		//instead of shifting, colorDepthConversionFactor was not divided by 256
		((uint32_t *)pixels)[i] = syncBits |
			((staticthis->colorDepthConversionFactor*(int)line[i * 2 + 1]) & 0xff00)
			| (((staticthis->colorDepthConversionFactor*(int)line[i * 2]) & 0xff00) << 16);
	}
}
