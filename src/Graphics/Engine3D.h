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
#include <stdlib.h>
#include "TriangleTree.h"
#include "../Tools/Log.h"
template<class Graphics>
class Engine3D
{
	public:
	typedef typename Graphics::Color Color;
	TriangleTree *triangleBuffer;
	TriangleTree *triangleRoot;
	int trinagleBufferSize;
	int triangleCount;

	Engine3D(const int initialTrinagleBufferSize = 1)
	{
		trinagleBufferSize = initialTrinagleBufferSize;
		triangleBuffer = (TriangleTree*)malloc(sizeof(TriangleTree) * trinagleBufferSize);
		if(!triangleBuffer)
			ERROR("Not enough memory for triangleBuffer");
		triangleRoot = 0;
		triangleCount = 0;
	}


	void enqueueTriangle(short *v0, short *v1, short *v2, Color color)
	{
		if (triangleCount >= trinagleBufferSize)
			return;
		TriangleTree &t = triangleBuffer[triangleCount++];
		t.set(v0, v1, v2, color);
		if (triangleRoot)
			triangleRoot->add(&triangleRoot, t);
		else
			triangleRoot = &t;
	}
	
	void drawTriangleTree(Graphics &g, TriangleTree *t)
	{
		if (t->left)
			drawTriangleTree(g, t->left);
		g.triangle(t->v[0], t->v[1], t->v[2], t->color);
		if (t->right)
			drawTriangleTree(g, t->right);
	}

	virtual void begin()
	{
		triangleCount = 0;
		triangleRoot = 0;
	}

	virtual void end(Graphics &g)
	{
		if (triangleRoot)
			drawTriangleTree(g, triangleRoot);
	}
};
