#pragma once
#include "Graphics.h"
#include "GameEntity.h"

class Animation: public GameEntity
{
  public:
  int start, end, frameDuration;
  int time;
  int drawMode;
  Animation(Sprites &sprites, int x, int y, int start, int end, int frameDuration, int drawMode = 0)
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
    time +=dt;
    return time < (end - start + 1) * frameDuration;
  }

  void draw(Graphics &g)
  {
    int current = time / frameDuration + start;
    if(drawMode == 0)
      sprites->drawMix(g, current, x, y);
    else if(drawMode == 1)
      sprites->drawAdd(g, current, x, y);
  }
};

Animation *animations[100] = {0};

void animationsAct(int dt)
{
   for(int i = 0; i < 100; i++)
  {
    if(animations[i])
      if(!animations[i]->act(dt))
      {
        delete animations[i];
        animations[i] = 0;
      }
  }
}

void animationsDraw()
{
    for(int i = 0; i < 100; i++)
    if(animations[i])
      if(animations[i])
        animations[i]->draw(graphics);
}

void animationsEmit(Animation *e)
{
  for(int i = 0; i < 100; i++)
  {
    if(!animations[i])
    {
      animations[i] = e;
      return;
    }
  }
  delete e;
}

