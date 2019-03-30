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

class Font
{
  public:
	const int firstChar;
	const int charCount;
	const unsigned char *pixels;
	const int charWidth;
	const int charHeight;
	Font(int charWidth, int charHeight, const unsigned char *pixels, int firstChar = 32, int charCount = 96)
		:firstChar(firstChar),
		charCount(charCount),
		pixels(pixels),
		charWidth(charWidth),
		charHeight(charHeight)
	{
	}

	bool valid(char ch) const
	{
		return ch >= firstChar && ch < firstChar + charCount;
	}
};
