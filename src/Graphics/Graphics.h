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
#include <stdlib.h>
#include <math.h>
#include "Font.h"
#include "Image.h"
#include "Entity.h"
#include "Sprites.h"

template<typename Color>
class Graphics
{
  public:
	typedef ImageT<Color> Image;
	typedef SpriteT<Color> Sprite;
	typedef SpritesT<Color> Sprites;

	int cursorX, cursorY, cursorBaseX;
	long frontColor, backColor;
	Font *font;
	int frameBufferCount;
	int currentFrameBuffer;
	Color **frameBuffers[3];
	Color **frontBuffer;
	Color **backBuffer;
	bool autoScroll;

	int xres;
	int yres;

	virtual void dotFast(int x, int y, Color color) = 0;
	virtual void dot(int x, int y, Color color) = 0;
	virtual void dotAdd(int x, int y, Color color) = 0;
	virtual void dotMix(int x, int y, Color color) = 0;
	virtual char get(int x, int y) = 0;
	virtual Color** allocateFrameBuffer() = 0;
	virtual Color RGBA(int r, int g, int b, int a = 255) const = 0;
	virtual int R(Color c) const = 0;
	virtual int G(Color c) const = 0;
	virtual int B(Color c) const = 0;
	virtual int A(Color c) const = 0;
	Color RGB(unsigned long rgb) const 
	{
		return RGBA(rgb & 255, (rgb >> 8) & 255, (rgb >> 16) & 255);
	}
	Color RGBA(unsigned long rgba) const 
	{
		return RGBA(rgba & 255, (rgba >> 8) & 255, (rgba >> 16) & 255, rgba >> 24);
	}
	Color RGB(int r, int g, int b) const 
	{
		return RGBA(r, g, b);
	}

	void setFrameBufferCount(unsigned char i)
	{
		frameBufferCount = i > 3 ? 3 : i;
	}

	virtual void show()
	{
		if(!frameBufferCount)
			return;
		currentFrameBuffer = (currentFrameBuffer + 1) % frameBufferCount;
		frontBuffer = frameBuffers[currentFrameBuffer];
		backBuffer = frameBuffers[(currentFrameBuffer + frameBufferCount - 1) % frameBufferCount];
	}

	Graphics(int xres = 0, int yres = 0)
	{
		this->xres = xres;
		this->yres = yres;
		font = 0;
		cursorX = cursorY = cursorBaseX = 0;
		frontColor = 0;
		backColor = 0;
		frameBufferCount = 1;
		for(int i = 0; i < 3; i++)
			frameBuffers[i] = 0;
		frontBuffer = 0;
		backBuffer = 0;
		autoScroll = true;
	}

	virtual bool allocateFrameBuffers()
	{
		if(yres <= 0 || xres <= 0)
			return false;

		for(int i = 0; i < frameBufferCount; i++)
			frameBuffers[i] = allocateFrameBuffer();
		currentFrameBuffer = 0;
		show();
		return true;
	}

	virtual void setResolution(int xres, int yres)
	{
		this->xres = xres;
		this->yres = yres;
		allocateFrameBuffers();
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

	void setFont(Font &font)
	{
		this->font = &font;
	}

	void setCursor(int x, int y)
	{
		cursorX = cursorBaseX = x;
		cursorY = y;
	}

	virtual void drawChar(int x, int y, int ch)
	{
		if (!font)
			return;
		const unsigned char *pix = &font->pixels[font->charWidth * font->charHeight * (ch - font->firstChar)];
		for (int py = 0; py < font->charHeight; py++)
			for (int px = 0; px < font->charWidth; px++)
				if (*(pix++))
					dot(px + x, py + y, frontColor);
				else if (backColor >= 0)
					dot(px + x, py + y, backColor);
	}

	void print(const char *str)
	{
		if (!font)
			return;
		while (*str)
		{
			if (font->valid(*str))
				drawChar(cursorX, cursorY, *str);
			cursorX += font->charWidth;
			if (cursorX + font->charWidth > xres || *str == '\n')
			{
				cursorX = cursorBaseX;
				cursorY += font->charHeight;
				if(autoScroll && cursorY + font->charHeight > yres)
					scroll(cursorY + font->charHeight - yres, backColor);
			}
			str++;
		}
	}

	void println(const char *str)
	{
		print(str); 
		print("\n");
	}

	void print(long number, int base = 10, int minCharacters = 1)
	{
		if(minCharacters < 1)
			minCharacters = 1;
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

	void println(long number, int base = 10, int minCharacters = 1)
	{
		print(number, base, minCharacters); print("\n");
	}

	void print(int number, int base = 10, int minCharacters = 1)
	{
		print(long(number), base, minCharacters);
	}

	void println(int number, int base = 10, int minCharacters = 1)
	{
		println(long(number), base, minCharacters);
	}

	void print(double number, int fractionalDigits = 2, int minCharacters = 1)
	{
		long p = long(pow(10, fractionalDigits));
		long long n = (double(number) * p + 0.5f);
		print(long(n / p), 10, minCharacters - 1 - fractionalDigits);
		if(fractionalDigits)
		{
			print(".");
			for(int i = 0; i < fractionalDigits; i++)
			{
				p /= 10;
				print(long(n / p) % 10);
			}
		}
	}

	void println(double number, int fractionalDigits = 2, int minCharacters = 1)
	{
		print(number, fractionalDigits, minCharacters); 
		print("\n");
	}

	virtual void clear(Color clear = 0)
	{
		for (int y = 0; y < yres; y++)
			for (int x = 0; x < xres; x++)
				dotFast(x, y, clear);
	}

	virtual void xLine(int x0, int x1, int y, Color color)
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

	void fillRect(int x, int y, int w, int h, Color color)
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

	void rect(int x, int y, int w, int h, Color color)
	{
		fillRect(x, y, w, 1, color);
		fillRect(x, y, 1, h, color);
		fillRect(x, y + h - 1, w, 1, color);
		fillRect(x + w - 1, y, 1, h, color);
	}

	void circle(int x, int y, int r, Color color)
	{
		int oxr = r;
		for(int i = 0; i < r + 1; i++)
		{
			int xr = (int)sqrt(r * r - i * i);
			xLine(x - oxr, x - xr + 1, y + i, color);
			xLine(x + xr, x + oxr + 1, y + i, color);
			if(i) 
			{
				xLine(x - oxr, x - xr + 1, y - i, color);
				xLine(x + xr, x + oxr + 1, y - i, color);
			}
			oxr = xr;
		}
	}

	void fillCircle(int x, int y, int r, Color color)
	{
		for(int i = 0; i < r + 1; i++)
		{
			int xr = (int)sqrt(r * r - i * i);
			xLine(x - xr, x + xr + 1, y + i, color);
			if(i) 
				xLine(x - xr, x + xr + 1, y - i, color);
		}
	}

	void ellipse(int x, int y, int rx, int ry, Color color)
	{
		int oxr = rx;
		float f = float(rx) / ry;
		f *= f;
		for(int i = 0; i < ry + 1; i++)
		{
			float s = rx * rx - i * i * f;
			int xr = (int)sqrt(s <= 0 ? 0 : s);
			xLine(x - oxr, x - xr + 1, y + i, color);
			xLine(x + xr, x + oxr + 1, y + i, color);
			if(i) 
			{
				xLine(x - oxr, x - xr + 1, y - i, color);
				xLine(x + xr, x + oxr + 1, y - i, color);
			}
			oxr = xr;
		}
	}

	void fillEllipse(int x, int y, int rx, int ry, Color color)
	{
		float f = float(rx) / ry;
		f *= f;		
		for(int i = 0; i < ry + 1; i++)
		{
			float s = rx * rx - i * i * f;
			int xr = (int)sqrt(s <= 0 ? 0 : s);
			xLine(x - xr, x + xr + 1, y + i, color);
			if(i) 
				xLine(x - xr, x + xr + 1, y - i, color);
		}
	}

	virtual void scroll(int dy, Color color)
	{
		//todo improve that
		if(dy > 0)
		{
			for(int d = 0; d < dy; d++)
			{
				Color *l = backBuffer[0];
				for(int i = 0; i < yres - 1; i++)
				{
					backBuffer[i] = backBuffer[i + 1];
				}
				backBuffer[yres - 1] = l;
				xLine(0, xres, yres - 1, color);
			}
		}
		else
		{
			for(int d = 0; d < -dy; d++)
			{
				Color *l = backBuffer[yres - 1];
				for(int i = 1; i < yres; i++)
				{
					backBuffer[i] = backBuffer[i - 1];
				}
				backBuffer[0] = l;
				xLine(0, xres, 0, color);
			}
		}
		cursorY -= dy;
	}	
};
