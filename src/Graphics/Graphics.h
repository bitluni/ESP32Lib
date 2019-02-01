#pragma once
#include "Font.h"
#include "TriangleTree.h"
#include <stdlib.h>
#include <math.h>

template <typename Color>
class Graphics
{
  public:
	int xres;
	int yres;
	Color **frame;
	Color **backbuffer;
	char **zbuffer;
	int cursorX, cursorY, cursorBaseX;
	long frontColor, backColor;
	Font<Graphics> *font;

	TriangleTree<Graphics<Color>, Color> *triangleBuffer;
	TriangleTree<Graphics<Color>, Color> *triangleRoot;
	int trinagleBufferSize;
	int triangleCount;

	Graphics()
	{
		font = 0;
		cursorX = cursorY = cursorBaseX = 0;
		triangleCount = 0;
		frontColor = 0;
		backColor = 0;
	}

	virtual float pixelAspect() const
	{
		return 1;
	}

	void setTextColor(long front, long back = -1)
	{
		//-1 = transparent back;
		frontColor = front;
		backColor = back;
	}

	virtual void initBuffers(const int w, const int h, const int initialTrinagleBufferSize = 0, const bool doubleBuffer = true)
	{
		xres = w;
		yres = h;
		trinagleBufferSize = initialTrinagleBufferSize;
		frame = (Color **)malloc(yres * sizeof(Color *));
		if(doubleBuffer)
			backbuffer = (Color **)malloc(yres * sizeof(Color *));
		else
			backbuffer = frame;
		//not enough memory for z-buffer implementation
		//zbuffer = (char**)malloc(yres * sizeof(char*));
		for (int y = 0; y < yres; y++)
		{
			frame[y] = (Color *)malloc(xres * sizeof(Color));
			if(doubleBuffer)
				backbuffer[y] = (Color *)malloc(xres * sizeof(Color));
			//zbuffer[y] = (char*)malloc(xres);
		}
		triangleBuffer = (TriangleTree<Graphics<Color>, Color> *)malloc(sizeof(TriangleTree<Graphics<Color>, Color>) * trinagleBufferSize);
	}

	void setFont(Font<Graphics> &font)
	{
		this->font = &font;
	}

	void setCursor(int x, int y)
	{
		cursorX = cursorBaseX = x;
		cursorY = y;
	}

	void print(const char *str)
	{
		if (!font)
			return;
		while (*str)
		{
			if (*str >= 32 && *str < 128)
				font->drawChar(*this, cursorX, cursorY, *str, frontColor, backColor);
			cursorX += font->xres;
			if (cursorX + font->xres > xres || *str == '\n')
			{
				cursorX = cursorBaseX;
				cursorY += font->yres;
			}
			str++;
		}
	}

	void print(int number, int base = 10, int minCharacters = 1)
	{
		bool sign = number < 0;
		if (sign)
			number = -number;
		const char baseChars[] = "0123456789ABCDEF";
		char temp[33];
		temp[32] = 0;
		int i = 31;
		do
		{
			temp[i--] = baseChars[number % base];
			number /= base;
		} while (number > 0);
		if (sign)
			temp[i--] = '-';
		for (; i > 31 - minCharacters; i--)
			temp[i] = ' ';
		print(&temp[i + 1]);
	}

	inline void clear(Color clear = 0)
	{
		for (int y = 0; y < yres; y++)
			for (int x = 0; x < xres; x++)
				backbuffer[y][x] = clear;
	}
	inline void begin()
	{
		triangleCount = 0;
		triangleRoot = 0;
	}

	inline void dotFast(int x, int y, Color color)
	{
		backbuffer[y][x] = color;
	}

	inline void dot(int x, int y, Color color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backbuffer[y][x] = color;
	}

	inline void dotAdd(int x, int y, Color color)
	{
		//todo repair this
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			backbuffer[y][x] = color + backbuffer[y][x];
	}
	
	inline void dotMix(int x, int y, unsigned int color)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
		{
			unsigned int ai = (3 - (color >> 14)) * (65536 / 3);
			unsigned int a = 65536 - ai;
			unsigned int co = backbuffer[y][x];
			unsigned int ro = (co & 0b11111) * ai;
			unsigned int go = (co & 0b1111100000) * ai;
			unsigned int bo = (co & 0b11110000000000) * ai;
			unsigned int r = (color & 0b11111) * a + ro;
			unsigned int g = ((color & 0b1111100000) * a + go) & 0b11111000000000000000000000;
			unsigned int b = ((color & 0b11110000000000) * a + bo) & 0b111100000000000000000000000000;
			backbuffer[y][x] = (r | g | b) >> 16;
		}	
	}	

	inline char get(int x, int y)
	{
		if ((unsigned int)x < xres && (unsigned int)y < yres)
			return backbuffer[y][x];
		return 0;
	}

	inline void xLine(int x0, int x1, int y, Color color)
	{
		if (x0 > x1)
		{
			int xb = x0;
			x0 = x1;
			x1 = xb;
		}
		if (x0 < 0)
			x0 = 0;
		if (x1 > xres)
			x1 = xres;
		for (int x = x0; x < x1; x++)
			dotFast(x, y, color);
	}

	void enqueueTriangle(short *v0, short *v1, short *v2, Color color)
	{
		if (triangleCount >= trinagleBufferSize)
			return;
		TriangleTree<Graphics, Color> &t = triangleBuffer[triangleCount++];
		t.set(v0, v1, v2, color);
		if (triangleRoot)
			triangleRoot->add(&triangleRoot, t);
		else
			triangleRoot = &t;
	}

	void triangle(short *v0, short *v1, short *v2, Color color)
	{
		short *v[3] = {v0, v1, v2};
		if (v[1][1] < v[0][1])
		{
			short *vb = v[0];
			v[0] = v[1];
			v[1] = vb;
		}
		if (v[2][1] < v[1][1])
		{
			short *vb = v[1];
			v[1] = v[2];
			v[2] = vb;
		}
		if (v[1][1] < v[0][1])
		{
			short *vb = v[0];
			v[0] = v[1];
			v[1] = vb;
		}
		int y = v[0][1];
		int xac = v[0][0] << 16;
		int xab = v[0][0] << 16;
		int xbc = v[1][0] << 16;
		int xaci = 0;
		int xabi = 0;
		int xbci = 0;
		if (v[1][1] != v[0][1])
			xabi = ((v[1][0] - v[0][0]) << 16) / (v[1][1] - v[0][1]);
		if (v[2][1] != v[0][1])
			xaci = ((v[2][0] - v[0][0]) << 16) / (v[2][1] - v[0][1]);
		if (v[2][1] != v[1][1])
			xbci = ((v[2][0] - v[1][0]) << 16) / (v[2][1] - v[1][1]);

		for (; y < v[1][1] && y < yres; y++)
		{
			if (y >= 0)
				xLine(xab >> 16, xac >> 16, y, color);
			xab += xabi;
			xac += xaci;
		}
		for (; y < v[2][1] && y < yres; y++)
		{
			if (y >= 0)
				xLine(xbc >> 16, xac >> 16, y, color);
			xbc += xbci;
			xac += xaci;
		}
	}

	void line(int x1, int y1, int x2, int y2, Color color)
	{
		int x, y, xe, ye;
		int dx = x2 - x1;
		int dy = y2 - y1;
		int dx1 = labs(dx);
		int dy1 = labs(dy);
		int px = 2 * dy1 - dx1;
		int py = 2 * dx1 - dy1;
		if (dy1 <= dx1)
		{
			if (dx >= 0)
			{
				x = x1;
				y = y1;
				xe = x2;
			}
			else
			{
				x = x2;
				y = y2;
				xe = x1;
			}
			dot(x, y, color);
			for (int i = 0; x < xe; i++)
			{
				x = x + 1;
				if (px < 0)
				{
					px = px + 2 * dy1;
				}
				else
				{
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
					{
						y = y + 1;
					}
					else
					{
						y = y - 1;
					}
					px = px + 2 * (dy1 - dx1);
				}
				dot(x, y, color);
			}
		}
		else
		{
			if (dy >= 0)
			{
				x = x1;
				y = y1;
				ye = y2;
			}
			else
			{
				x = x2;
				y = y2;
				ye = y1;
			}
			dot(x, y, color);
			for (int i = 0; y < ye; i++)
			{
				y = y + 1;
				if (py <= 0)
				{
					py = py + 2 * dx1;
				}
				else
				{
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
					{
						x = x + 1;
					}
					else
					{
						x = x - 1;
					}
					py = py + 2 * (dx1 - dy1);
				}
				dot(x, y, color);
			}
		}
	}

	inline void flush()
	{
		if (triangleRoot)
			triangleRoot->draw(*this);
	}

	inline void end()
	{
		Color **b = backbuffer;
		backbuffer = frame;
		frame = b;
	}

	void fillRect(int x, int y, int w, int h, int color)
	{
		if (x < 0)
		{
			w += x;
			x = 0;
		}
		if (y < 0)
		{
			h += y;
			y = 0;
		}
		if (x + w > xres)
			w = xres - x;
		if (y + h > yres)
			h = yres - y;
		for (int j = y; j < y + h; j++)
			for (int i = x; i < x + w; i++)
				dotFast(i, j, color);
	}

	void rect(int x, int y, int w, int h, int color)
	{
		fillRect(x, y, w, 1, color);
		fillRect(x, y, 1, h, color);
		fillRect(x, y + h - 1, w, 1, color);
		fillRect(x + w - 1, y, 1, h, color);
	}
};
