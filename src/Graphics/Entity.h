#pragma once
#include "Graphics.h"

class Entity
{
  public:
  int x, y;
  int vx, vy;
  int life;
  int faction;
  Sprites *sprites;
  virtual bool act(int dt);
  virtual void draw(Graphics &g);
};

