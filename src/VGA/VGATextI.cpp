/*
	Author: Martin-Laclaustra 2021
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
*/

#include <VGA/VGATextI.h>

void IRAM_ATTR VGATextI::interrupt(void *arg)
{
	VGATextI * staticthis = (VGATextI *)arg;

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
		if (y >= 0 && y < (staticthis->mode.vRes / staticthis->mode.vDiv) )
			staticthis->interruptPixelLine(y, activeRenderingBuffer, arg);
	}

	if (renderLine == 0)
		staticthis->vSyncPassed = true;
}

	//LOWER LIMIT: THE CODE BETWEEN THESE MARKS IS SHARED BETWEEN 3BIT, 6BIT, AND 14BIT

void IRAM_ATTR VGATextI::interruptPixelLine(int y, uint8_t *pixels8, void *arg)
{
	VGATextI * staticthis = (VGATextI *)arg;
	unsigned long syncBits = (staticthis->hsyncBitI | staticthis->vsyncBitI) * staticthis->rendererStaticReplicate32mask;
	uint32_t * pixels = (uint32_t *)pixels8;

	unsigned char *charline = staticthis->frontBuffer[y/staticthis->font->charHeight];
	unsigned char charactersubline = y % staticthis->font->charHeight;

	uint32_t pixel = 0;
	uint8_t * pixelbyte = (uint8_t *)&pixel;
	unsigned char renderchar = 0;
	const uint8_t *line = &staticthis->font->pixels[staticthis->font->charWidth * charactersubline];
	const uint8_t *lineoffset = 0;
	const uint8_t characterinterval = staticthis->font->charWidth * staticthis->font->charHeight;


	//pixel in row
	int i = 0;
	//which pixel column within the character
	int xcolumn = 0;
	renderchar = *charline;
	if (!staticthis->font->valid(renderchar)) renderchar = ' ';
	lineoffset = &line[xcolumn + characterinterval * (renderchar - staticthis->font->firstChar)];
	while (i < staticthis->mode.hRes)
	{
		if(staticthis->font->charWidth - xcolumn >=4)
		{
			pixelbyte[2] = *lineoffset++;
			pixelbyte[3] = *lineoffset++;
			pixelbyte[0] = *lineoffset++;
			pixelbyte[1] = *lineoffset++;
			if((xcolumn += 4) >= staticthis->font->charWidth)
			{
				xcolumn=0;
				renderchar = *++charline;
				if (!staticthis->font->valid(renderchar)) renderchar = ' ';
				lineoffset = &line[characterinterval * (renderchar - staticthis->font->firstChar)];
			}
		} else {
			switch(staticthis->font->charWidth - xcolumn)
			{
				case 1:
					pixelbyte[2] = *lineoffset++;
					renderchar = *++charline;
					if (!staticthis->font->valid(renderchar)) renderchar = ' ';
					lineoffset = &line[characterinterval * (renderchar - staticthis->font->firstChar)];
					pixelbyte[3] = *lineoffset++;
					pixelbyte[0] = *lineoffset++;
					pixelbyte[1] = *lineoffset++;
					xcolumn=3;
					break;
				case 2:
					pixelbyte[2] = *lineoffset++;
					pixelbyte[3] = *lineoffset++;
					renderchar = *++charline;
					if (!staticthis->font->valid(renderchar)) renderchar = ' ';
					lineoffset = &line[characterinterval * (renderchar - staticthis->font->firstChar)];
					pixelbyte[0] = *lineoffset++;
					pixelbyte[1] = *lineoffset++;
					xcolumn=2;
					break;
				case 3:
					pixelbyte[2] = *lineoffset++;
					pixelbyte[3] = *lineoffset++;
					pixelbyte[0] = *lineoffset++;
					renderchar = *++charline;
					if (!staticthis->font->valid(renderchar)) renderchar = ' ';
					lineoffset = &line[characterinterval * (renderchar - staticthis->font->firstChar)];
					pixelbyte[1] = *lineoffset++;
					xcolumn=1;
					break;
			}
		}

		pixel &= 0x01010101;
		*pixels++ = syncBits
		 | (pixel * staticthis->frontGlobalColor)
		 | ((pixel^0x01010101) * staticthis->backGlobalColor);
		i+=4;
	}
}
