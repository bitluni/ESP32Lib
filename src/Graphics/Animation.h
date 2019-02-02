/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 2.0
	https://creativecommons.org/licenses/by-sa/2.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once
#include "Graphics.h"
#include "Entity.h"

template<typename Color>
class Animation : public Entity
{
  public:
	int start, end, frameDuration;
	int time;
	int drawMode;
	Animation(Sprites<Color> &sprites, int x, int y, int start, int end, int frameDuration, int drawMode = 0)
	{
		this->x = x;
		this->y = y;
		this->sprites = &sprites;
		this->start = start;
		this->end = end;
		this->frameDuration = frameDuration;
		time = 0;
		this->drawMode = drawMode;
	}

	bool act(int dt)
	{
		time += dt;
		return time < (end - start + 1) * frameDuration;
	}

	void draw(Graphics<Color> &g)
	{
		int current = time / frameDuration + start;
		if (drawMode == 0)
			sprites->drawMix(g, current, x, y);
		else if (drawMode == 1)
			sprites->drawAdd(g, current, x, y);
	}
};

template<typename Color>
void animationsAct(Animation<Color> *animations, int dt, int maxCount = 100)
{
	for (int i = 0; i < maxCount; i++)
	{
		if (animations[i])
			if (!animations[i]->act(dt))
			{
				delete animations[i];
				animations[i] = 0;
			}
	}
}

template<typename Color>
void animationsDraw(Animation<Color> *animations, int maxCount = 100)
{
	for (int i = 0; i < maxCount; i++)
		if (animations[i])
			if (animations[i])
				animations[i]->draw(graphics);
}

template<typename Color>
void animationsEmit(Animation<Color> *animations, Animation<Color> *e, int maxCount = 100)
{
	for (int i = 0; i < maxCount; i++)
	{
		if (!animations[i])
		{
			animations[i] = e;
			return;
		}
	}
	delete e;
}
