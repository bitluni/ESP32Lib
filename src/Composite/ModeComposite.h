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

class ModeComposite
{
  public:
	int hSync;
	int hFront;
	int hRes;
	int hBack;
	int vSync;
	int vFront;
	int vRes;
	int vBack;
	int vDiv;
	int shortSync;
	unsigned long pixelClock;
	int burstStart;
	int burstLength;
	unsigned long colorClock;
	float aspect;
	int activeLineCount;
	ModeComposite(
		const int hSync = 0,
		const int hFront = 0,
		const int hRes = 0,
		const int hBack = 0,
		const int vSync = 0,
		const int vFront = 0,
		const int vRes = 0,
		const int vBack = 0,
		const int vDiv = 1,
		const int shortSync = 0,
		const unsigned long pixelClock = 0,
		const int burstStart = 0,
		const int burstLength = 0,
		const unsigned long colorClock = 0,
		const float aspect = 1.f)
		:
		  hSync(hSync),
		  hFront(hFront),
		  hRes(hRes),
		  hBack(hBack),
		  vSync(vSync),
		  vFront(vFront),
		  vRes(vRes),
		  vBack(vBack),
		  vDiv(vDiv),
		  shortSync(shortSync),
		  pixelClock(pixelClock),
		  burstStart(burstStart),
		  burstLength(burstLength),
		  colorClock(colorClock),
		  aspect(aspect),
		  activeLineCount(vRes / vDiv)
	{
	}

	int linesPerField() const
	{
		return vSync + vFront + vBack + vRes;
	}

	int pixelsPerLine() const
	{
		return hFront + hSync + hBack + hRes;
	}

	ModeComposite custom(int xres, int yres, int fixedYDivider = 0) const
	{
		/*xres = (xres + 3) & 0xfffffffc;
		float f = float(xres) / hRes;
		int hs = int(hSync * f + 3) & 0xfffffffc;
		int hb = int((hSync + hBack - hs / f) * f + 3) & 0xfffffffc;
		int hr = xres;
		int hf = int((pixelsPerLine() - (hs + hb + hr) / f) * f + 3) & 0xfffffffc;
		
		int vd = fixedYDivider ? fixedYDivider : (vRes / yres);
		int vr = yres * vd;
		int vf = vFront + vRes / 2 - vr / 2;
		int vb = vBack + vRes / 2 - (vr - vr / 2);
		long pc = long(pixelClock * f);
		return ModeComposite(hf, hs, hb, hr, vf, vSync, vb, vr, vd, pc, hSyncPolarity, vSyncPolarity);*/
		return *this;
	}

	template<class Output>
	void print(Output &output) const
	{
		output.print("hSync: ");
 		output.println(hSync);
		output.print("hFront: ");
		output.println(hFront);
		output.print("hRes: ");
		output.println(hRes);
		output.print("hBack: ");
		output.println(hBack);
		output.print("vSync: ");
		output.println(vSync);
		output.print("vFront: ");
		output.println(vFront);
		output.print("vBack: ");
		output.println(vBack);
		output.print("vRes: ");
		output.println(vRes);
		output.print("vDiv: ");
		output.println(vDiv);
		output.print("shortSync: ");
		output.println(shortSync);
		output.print("pixelClock: ");
		output.println(pixelClock);
		output.print("burstStart: ");
		output.println(burstStart);
		output.print("burstLength: ");
		output.println(burstLength);
		output.print("colorClock: ");
		output.println(colorClock);
	}
};