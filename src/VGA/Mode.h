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

class Mode
{
  public:
	int hFront;
	int hSync;
	int hBack;
	int hRes;
	int vFront;
	int vSync;
	int vBack;
	int vRes;
	int vDiv;
	unsigned long pixelClock;
	int hSyncPolarity;
	int vSyncPolarity;
	float aspect;
	int activeLineCount; // calculated: actual information for lines displayed
	Mode(
		const int hFront = 0,
		const int hSync = 0,
		const int hBack = 0,
		const int hRes = 0,
		const int vFront = 0,
		const int vSync = 0,
		const int vBack = 0,
		const int vRes = 0,
		const int vDiv = 1,
		const unsigned long pixelClock = 0,
		const int hSyncPolarity = 1,
		const int vSyncPolarity = 1,
		const float aspect = 1.f)
		: hFront(hFront),
		  hSync(hSync),
		  hBack(hBack),
		  hRes(hRes),
		  vFront(vFront),
		  vSync(vSync),
		  vBack(vBack),
		  vRes(vRes),
		  vDiv(vDiv),
		  pixelClock(pixelClock),
		  hSyncPolarity(hSyncPolarity),
		  vSyncPolarity(vSyncPolarity),
		  aspect(aspect),
		  activeLineCount(vRes / vDiv)
	{
	}

	int maxXRes() const
	{
		return (int(hRes * 19673499. / pixelClock) & 0xfffffffe);
	}

	int linesPerField() const
	{
		return vFront + vSync + vBack + vRes;
	}

	int pixelsPerLine() const
	{
		return hFront + hSync + hBack + hRes;
	}

	Mode custom(int xres, int yres, int fixedYDivider = 0) const
	{
		xres = (xres + 3) & 0xfffffffc;
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
		return Mode(hf, hs, hb, hr, vf, vSync, vb, vr, vd, pc, hSyncPolarity, vSyncPolarity);
	}

	template<class Output>
	void print(Output &output) const
	{
		output.print("hFront: ");
		output.println(hFront);
		output.print("hSync: ");
 		output.println(hSync);
		output.print("hBack: ");
		output.println(hBack);
		output.print("hRes: ");
		output.println(hRes);
		output.print("vFront: ");
		output.println(vFront);
		output.print("vSync: ");
		output.println(vSync);
		output.print("vBack: ");
		output.println(vBack);
		output.print("vRes: ");
		output.println(vRes);
		output.print("vDiv: ");
		output.println(vDiv);
		output.print("pixelClock: ");
		output.println(pixelClock);
		output.print("hSyncPolarity: ");
		output.println(hSyncPolarity);
		output.print("vSyncPolarity: ");
		output.println(vSyncPolarity);
		output.print("hfreq: ");
		output.println((double)pixelClock/(double)pixelsPerLine());
		output.print("vfreq: ");
		output.println((double)pixelClock/((double)pixelsPerLine()*(double)linesPerField()));
	}
};
