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

class TriangleTree
{
  public:
	short *v[3];
	int z;
	TriangleTree *left, *right;
	int depth;
	long color;

	void set(short *v0, short *v1, short *v2, long color)
	{
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		z = v[0][2] + v[1][2] + v[2][2];
		this->color = color;
		left = right = 0;
		depth = 1;
	}

	int leftDepth() const
	{
		return left ? left->depth : 0;
	}

	int rightDepth() const
	{
		return right ? right->depth : 0;
	}

	void recalcDepth()
	{
		int l = leftDepth();
		int r = rightDepth();
		depth = l > r ? l : r;
	}

	int add(TriangleTree **origin, TriangleTree &t)
	{
		int d = 1;
		if (t.z < z)
		{
			if (left)
				d = left->add(&left, t);
			else
				left = &t;
		}
		else
		{
			if (right)
				d = right->add(&right, t);
			else
				right = &t;
		}
		if (depth < d + 1)
			depth = d + 1;
		int l = leftDepth();
		int r = rightDepth();
		if (l > r + 1)
		{
			int ll = left->leftDepth();
			int lr = left->rightDepth();
			if (ll < lr)
			{
				TriangleTree *tl = left;
				left = tl->right;
				tl->right = left->left;
				left->left = tl;
				left->left->recalcDepth();
				left->recalcDepth();
				ll = left->leftDepth();
				lr = left->rightDepth();
				l = leftDepth();
				recalcDepth();
			}
			{
				*origin = left;
				left = left->right;
				(*origin)->right = this;
				depth = lr > r ? lr + 1 : r + 1;
				(*origin)->depth = ll > depth ? ll + 1 : depth + 1;
				return (*origin)->depth + 1;
			}
		}
		if (r > l + 1)
		{
			int rl = right->leftDepth();
			int rr = right->rightDepth();
			if (rr < rl)
			{
				TriangleTree *tr = right;
				right = tr->left;
				tr->left = right->right;
				right->right = tr;
				right->right->recalcDepth();
				right->recalcDepth();
				rr = right->rightDepth();
				rl = right->leftDepth();
				r = rightDepth();
				recalcDepth();
			}
			{
				*origin = right;
				right = right->left;
				(*origin)->left = this;
				depth = rl > l ? rl + 1 : l + 1;
				(*origin)->depth = rr > depth ? rr + 1 : depth + 1;
				return (*origin)->depth + 1;
			}
		}
		return depth;
	}
};
