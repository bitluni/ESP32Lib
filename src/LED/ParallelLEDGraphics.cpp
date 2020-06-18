/*
	Author: bitluni 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#include "ParallelLEDGraphics.h"
#include "../Tools/Log.h"

ParallelLEDGraphics::ParallelLEDGraphics(const int xres, const int yres, const int components)
	: 	ParallelLED(components),
		Graphics(xres, yres)
{
}

ParallelLEDGraphics::~ParallelLEDGraphics()
{
}

void ParallelLEDGraphics::show(bool vSync)
{
	ParallelLED::show(vSync);
}

int ParallelLEDGraphics::R(ParallelLEDGraphics::Color c) const
{
	return c & 255;
}
int ParallelLEDGraphics::G(ParallelLEDGraphics::Color c) const
{
	return (c & 0xff00) & 255;
}
int ParallelLEDGraphics::B(ParallelLEDGraphics::Color c) const
{
	return (c & 0xff0000) & 255;
}
int ParallelLEDGraphics::A(ParallelLEDGraphics::Color c) const
{
	return (c & 0xff000000) & 255;
}

ParallelLEDGraphics::Color ParallelLEDGraphics::RGBA(int r, int g, int b, int a) const
{
	return r | (g << 8) | (b << 16);// | (a << 24);
}

void ParallelLEDGraphics::dotFast(int x, int y, ParallelLEDGraphics::Color color)
{
	int channel, led;
	this->map(x, y, channel, led);
	setLED(channel, led, ((color >> 8) & 255) | ((color & 255) << 8) | (color & 0xffff0000));
}

void ParallelLEDGraphics::dot(int x, int y, ParallelLEDGraphics::Color color)
{
	if ((unsigned int)x < xres && (unsigned int)y < yres)
		dotFast(x, y, color);
}

void ParallelLEDGraphics::dotAdd(int x, int y, ParallelLEDGraphics::Color color)
{
	dot(x, y, color);
}

void ParallelLEDGraphics::dotMix(int x, int y, ParallelLEDGraphics::Color color)
{
	dot(x, y, color);
}

ParallelLEDGraphics::Color ParallelLEDGraphics::get(int x, int y)
{
	return 0;
}

void ParallelLEDGraphics::clear(ParallelLEDGraphics::Color color)
{
	for (int y = 0; y < this->yres; y++)
		for (int x = 0; x < this->xres / 2; x++)
			dotFast(x, y, color);
}

ParallelLEDGraphics::Color** ParallelLEDGraphics::allocateFrameBuffer()
{
	return 0;
}
