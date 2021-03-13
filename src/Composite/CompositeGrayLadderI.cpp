/*
	Author: Martin-Laclaustra 2021
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
	Custom ladders can be used by tweaking colorMinValue and colorMaxValue
*/

#include <Composite/CompositeGrayLadderI.h>

void IRAM_ATTR CompositeGrayLadderI::interrupt(void *arg)
{
	CompositeGrayLadderI * staticthis = (CompositeGrayLadderI *)arg;

	//obtain currently rendered line from the buffer just read, based on the conventioned ordering and buffers per line
	staticthis->currentLine = staticthis->dmaBufferDescriptorActive >> ( (staticthis->descriptorsPerLine==2) ? 1 : 0 );

	//in the case of two buffers per line,
	//render only when the sync half of the line ended (longer period until next interrupt)
	//else exit early
	//This might need to be revised, because it might be better to overlap and miss the second interrupt
	if ( (staticthis->descriptorsPerLine==2) && ((staticthis->dmaBufferDescriptorActive & 1) != 0) ) return;

	//render ahead (the lenght of buffered lines)
	int renderLine = (staticthis->currentLine + staticthis->lineBufferCount);
	if (renderLine >= staticthis->totalLines) renderLine -= staticthis->totalLines;

	if(!staticthis->mode.interlaced)
	{
		//TO DO: This should be precalculated outside the interrupt
		int vInactiveLinesCount = staticthis->mode.vFront + staticthis->mode.vOddFieldOffset + staticthis->mode.vBack;

		if (renderLine >= vInactiveLinesCount)
		{
			int renderActiveLine = renderLine - vInactiveLinesCount;
			uint8_t *activeRenderingBuffer = ((uint8_t *)
			staticthis->dmaBufferDescriptors[renderLine * staticthis->descriptorsPerLine + staticthis->descriptorsPerLine - 1].buffer() + staticthis->dataOffsetInLineInBytes
			);

			int y = renderActiveLine / staticthis->mode.vDiv;
			if (y >= 0 && y < staticthis->yres)
				staticthis->interruptPixelLine(y, activeRenderingBuffer, arg);
		}

		if (renderLine == 0)
			staticthis->vSyncPassed = true;
	} else {
		//TO DO: This should be precalculated outside the interrupt
		int oddFieldStart = staticthis->mode.vFront + staticthis->mode.vOddFieldOffset + staticthis->mode.vBack;
		int oddFieldEnd = oddFieldStart + staticthis->mode.vActive;
		int evenFieldStart = staticthis->mode.vFront + staticthis->mode.vEvenFieldOffset + staticthis->mode.vBack;
		int evenFieldEnd = evenFieldStart + staticthis->mode.vActive;
		
		if (renderLine >= oddFieldStart && renderLine < oddFieldEnd)
		{
			int renderActiveLine = renderLine - oddFieldStart;
			uint8_t *activeRenderingBuffer = ((uint8_t *)
			staticthis->dmaBufferDescriptors[renderLine * staticthis->descriptorsPerLine + staticthis->descriptorsPerLine - 1].buffer() + staticthis->dataOffsetInLineInBytes
			);

			int y = 2*renderActiveLine / staticthis->mode.vDiv;
			if (y >= 0 && y < staticthis->yres)
				staticthis->interruptPixelLine(y, activeRenderingBuffer, arg);
		} else if (renderLine >= evenFieldStart && renderLine < evenFieldEnd)
		{
			int renderActiveLine = renderLine - evenFieldStart;
			uint8_t *activeRenderingBuffer = ((uint8_t *)
			staticthis->dmaBufferDescriptors[renderLine * staticthis->descriptorsPerLine + staticthis->descriptorsPerLine - 1].buffer() + staticthis->dataOffsetInLineInBytes
			);

			int y = (2*renderActiveLine + 1) / staticthis->mode.vDiv;
			if (y >= 0 && y < staticthis->yres)
				staticthis->interruptPixelLine(y, activeRenderingBuffer, arg);
		}

		if (renderLine == 0)
			staticthis->vSyncPassed = true;
	}

}

void IRAM_ATTR CompositeGrayLadderI::interruptPixelLine(int y, uint8_t *pixels, void *arg)
{
	CompositeGrayLadderI * staticthis = (CompositeGrayLadderI *)arg;
	int p = staticthis->baseBufferValue << 8;
	int p0, p1, p2, p3;
	uint8_t *line = staticthis->frontBuffer[y];
	int j = 0;
	for (int i = 0; i < staticthis->mode.hRes / 4; i++)
	{
		//writing four pixels improves speed drastically (avoids memory reads)
		//instead of shifting, colorDepthConversionFactor was not divided by 256
		p0 = p1 = p2 = p3 = p;
		p0 += ((int)(line[j++]) * staticthis->colorDepthConversionFactor);
		p0 &= 0xff00;
		p1 += ((int)(line[j++]) * staticthis->colorDepthConversionFactor);
		p1 &= 0xff00;
		p2 += ((int)(line[j++]) * staticthis->colorDepthConversionFactor);
		p2 &= 0xff00;
		p3 += ((int)(line[j++]) * staticthis->colorDepthConversionFactor);
		p3 &= 0xff00;
		((uint32_t *)pixels)[i] = (p2 >> 8) | (p3 << 0) | (p0 << 8) | (p1 << 16);
	}
}
