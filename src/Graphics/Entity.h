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
#include "Sprites.h"

template<class Graphics>
class Entity
{
  public:
	int x, y;
	int vx, vy;
	int life;
	int faction;
	Sprites<Graphics> *sprites;
	virtual bool act(int dt);
	virtual void draw(Graphics &g);
};
