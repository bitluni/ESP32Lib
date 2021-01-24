/*
	Author: bitluni 2019 and Martin-Laclaustra 2020
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
	int hFront;
	int hSync;
	int hBack;
	int hRes;
	int vFront;    // All below: vertical lines per field (frame lines are double)
	int vPreEqHL;  // Pre-Equalizing Half Lines
	int vSyncHL;   // Sync Half Lines
	int vPostEqHL; // Post-Equalizing Half Lines
	int vBack;
	int vActive;
	int vOPreRegHL;  // for interlacing: Odd PreRegular HalfLines (BeforeEqPulses) NTSC 0  PAL 1 (first half of a regular line)
	int vOPostRegHL; // for interlacing: Odd PostRegular HalfLines (AfterEqPulses)      0      2 (first+second half of a regular line)
	int vEPreRegHL;  // for interlacing: Even PreRegular HalfLines (BeforeEqPulses)     1      0 (first half of a regular line)
	int vEPostRegHL; // for interlacing: Even PostRegular HalfLines (AfterEqPulses)     1      1 (second half of a regular line)
	int vDiv;
	unsigned long pixelClock;
	int burstStart;
	int burstLength;
	unsigned long colorClock;
	int phaseAlternating; // 0 for NTSC, 1 for PAL
	float aspect;
	int vRes; // calculated: total active lines per frame
	int activeLineCount; // calculated: actual information for lines displayed
	int vOddFieldOffset; // calculated: start of the odd field non-vsync lines
	int vEvenFieldOffset; // calculated: start of the even field non-vsync lines
	bool interlaced; // calculated: convenience boolean
	int linesPerFrame; // calculated: 
	//int linesPerField; // calculated: 
	int vSync; // calculated: (back compatibility)
	int shortSync; // calculated: (back compatibility)
	
	ModeComposite(
		const int hFront = 0,
		const int hSync = 0,
		const int hBack = 0,
		const int hRes = 0,
		const int vFront = 0,
		const int vPreEqHL = 0,
		const int vSyncHL = 0,
		const int vPostEqHL = 0,
		const int vBack = 0,
		const int vActive = 0,
		const int vOPreRegHL = 0,
		const int vOPostRegHL = 0,
		const int vEPreRegHL = 0,
		const int vEPostRegHL = 0,
		const int vDiv = 1,
		const unsigned long pixelClock = 0,
		const int burstStart = 0,
		const int burstLength = 0,
		const unsigned long colorClock = 0,
		const int phaseAlternating = 0,
		const float aspect = 1.f
		)
		:
		  hFront(hFront),
		  hSync(hSync),
		  hBack(hBack),
		  hRes(hRes),
		  vFront(vFront),
		  vPreEqHL(vPreEqHL),
		  vSyncHL(vSyncHL),
		  vPostEqHL(vPostEqHL),
		  vBack(vBack),
		  vActive(vActive),
		  vOPreRegHL(vOPreRegHL),
		  vOPostRegHL(vOPostRegHL),
		  vEPreRegHL(vEPreRegHL),
		  vEPostRegHL(vEPostRegHL),
		  vDiv(vDiv),
		  pixelClock(pixelClock),
		  burstStart(burstStart),
		  burstLength(burstLength),
		  colorClock(colorClock),
		  phaseAlternating(phaseAlternating),
		  aspect(aspect),
		  vRes(vActive*(((vOPreRegHL + vOPostRegHL + vEPreRegHL + vEPostRegHL) > 0)?2:1)),
		  activeLineCount( (vActive*(((vOPreRegHL + vOPostRegHL + vEPreRegHL + vEPostRegHL) > 0)?2:1)) / vDiv),
		  vOddFieldOffset((vOPreRegHL + vPreEqHL + vSyncHL + vPostEqHL + vOPostRegHL) / 2),
		  vEvenFieldOffset(((vOPreRegHL + vPreEqHL + vSyncHL + vPostEqHL + vOPostRegHL) / 2) + vBack + vActive + vFront + ((vEPreRegHL + vPreEqHL + vSyncHL + vPostEqHL + vEPostRegHL) / 2)),
		  interlaced((vOPreRegHL + vOPostRegHL + vEPreRegHL + vEPostRegHL) > 0),
		  linesPerFrame(
		  
		  (
			(
			(((vOPreRegHL + vOPostRegHL + vEPreRegHL + vEPostRegHL) > 0)?2:1) // fields per frame
			*
			(vFront + vBack + vActive) // lines in every field
			)
			+
			(
			( (((vOPreRegHL + vOPostRegHL + vEPreRegHL + vEPostRegHL) > 0)?2:1) // fields per frame
			*
			(vPreEqHL + vSyncHL + vPostEqHL) ) / 2
			)
			+
			(
			(vOPreRegHL + vOPostRegHL + vEPreRegHL + vEPostRegHL) / 2
			)
		  )
		  
		  ),
		  vSync((vPreEqHL + vSyncHL + vPostEqHL) / 2),
		  shortSync(hSync/2)
	{
	}

	int linesPerField() const // left for compatibility; this is a fractional number in interlaced modes
	{
		return linesPerFrame;
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
		output.print("vRes: ");
		output.println(vRes);
		output.print("activeLineCount: ");
		output.println(activeLineCount);
		output.print("vOddFieldOffset: ");
		output.println(vOddFieldOffset);
		output.print("vEvenFieldOffset: ");
		output.println(vEvenFieldOffset);
		output.print("interlaced: ");
		output.println(interlaced);
		output.print("linesPerFrame: ");
		output.println(linesPerFrame);
		output.print("linesPerField: ");
		output.println(linesPerField);
		output.print("hfreq: ");
		output.println((double)pixelClock/(double)pixelsPerLine());
		output.print("vfreq: ");
		output.println((double)pixelClock/((double)pixelsPerLine()*(double)linesPerFrame));
	}
};
