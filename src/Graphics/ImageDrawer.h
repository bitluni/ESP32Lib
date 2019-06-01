/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once
#include "Image.h"
class ImageDrawer
{
	public:
	virtual void imageR5G5B4A2(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres) = 0;
	virtual void imageAddR5G5B4A2(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres) = 0;
	virtual void imageMixR5G5B4A2(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres) = 0;
	virtual void imageR2G2B2A2(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres) = 0;
	virtual void imageAddR2G2B2A2(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres) = 0;
	virtual void imageMixR2G2B2A2(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres) = 0;

	void image(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres)
	{
		switch(image.pixelFormat)
		{
			case Image::R5G5B4A2:
				imageR5G5B4A2(image, x, y, srcX, srcY, srcXres, srcYres);
			break;
			case Image::R2G2B2A2:
				imageR2G2B2A2(image, x, y, srcX, srcY, srcXres, srcYres);
			break;
			default:
			break;
		}
	}

	void imageAdd(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres)
	{
		switch(image.pixelFormat)
		{
			case Image::R5G5B4A2:
				imageAddR5G5B4A2(image, x, y, srcX, srcY, srcXres, srcYres);
			break;
			case Image::R2G2B2A2:
				imageAddR2G2B2A2(image, x, y, srcX, srcY, srcXres, srcYres);
			break;
			default:
			break;
		}
	}

	void imageMix(Image &image, int x, int y, int srcX, int srcY, int srcXres, int srcYres)
	{
		switch(image.pixelFormat)
		{
			case Image::R5G5B4A2:
				imageMixR5G5B4A2(image, x, y, srcX, srcY, srcXres, srcYres);
			break;
			case Image::R2G2B2A2:
				imageMixR2G2B2A2(image, x, y, srcX, srcY, srcXres, srcYres);
			break;
			default:
			break;
		}
	}

	void image(Image &image, int x, int y)
	{
		switch(image.pixelFormat)
		{
			case Image::R5G5B4A2:
				imageR5G5B4A2(image, x, y, 0, 0, image.xres, image.yres);
			break;
			case Image::R2G2B2A2:
				imageR2G2B2A2(image, x, y, 0, 0, image.xres, image.yres);
			break;
			default:
			break;
		}
	}

	void imageAdd(Image &image, int x, int y)
	{
		switch(image.pixelFormat)
		{
			case Image::R5G5B4A2:
				imageAddR5G5B4A2(image, x, y, 0, 0, image.xres, image.yres);
			break;
			case Image::R2G2B2A2:
				imageAddR2G2B2A2(image, x, y, 0, 0, image.xres, image.yres);
			break;
			default:
			break;
		}
	}

	void imageMix(Image &image, int x, int y)
	{
		switch(image.pixelFormat)
		{
			case Image::R5G5B4A2:
				imageMixR5G5B4A2(image, x, y, 0, 0, image.xres, image.yres);
			break;
			case Image::R2G2B2A2:
				imageMixR2G2B2A2(image, x, y, 0, 0, image.xres, image.yres);
			break;
			default:
			break;
		}
	}
};