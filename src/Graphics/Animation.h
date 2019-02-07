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
#include "Entity.h"

template <typename Color = unsigned short>
class AnimationT : public EntityT<Color>
{
  public:
	int start, end, frameDuration;
	int time;
	int drawMode;

	AnimationT(SpritesT<Color> &sprites, int x, int y, int start, int end, int frameDuration, int drawMode = 0)
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
/*TODO
	void draw(Graphics &g)
	{
		int current = time / frameDuration + start;
		if (drawMode == 0)
			this->sprites->drawMix(g, current, this->x, this->y);
		else if (drawMode == 1)
			this->sprites->drawAdd(g, current, this->x, this->y);
	}	*/
};

template <typename Color = unsigned short>
void animationsAct(AnimationT<Color> **animations, int dt, int maxCount = 100)
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
/*TODO
template <typename Color = unsigned short>
void animationsDraw(Graphics &g, Animation<Color> **animations, int maxCount = 100)
{
	for (int i = 0; i < maxCount; i++)
		if (animations[i])
			if (animations[i])
				animations[i]->draw(g);
}
*/
template <typename Color = unsigned short>
void animationsEmit(AnimationT<Color> **animations, AnimationT<Color> *e, int maxCount = 100)
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
