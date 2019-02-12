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
#include "Graphics.h"
#include "Sprites.h"

class Entity
{
  public:
	int x, y;
	int vx, vy;
	int life;
	int faction;
	Sprites *sprites;
	virtual bool act(int dt);
	virtual void draw() = 0;
};
