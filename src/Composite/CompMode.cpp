/*
	Author: Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#include "CompMode.h"

//ModeComposite arguments order:
//==============================
//Horizontal pixels (in order):
//   hFront,hSync,hBack,hRes,
//Vertical lines or half-lines (in order):
//   vFront,vPreEqHL,vSyncHL,vPostEqHL,vBack,vActive,
//Vertical half-lines specific of odd(first) or even(second) fields:
//   vOPreRegHL,vOPostRegHL,vEPreRegHL,vEPostRegHL,
//Vertical resolution reduction:
//   vDiv,
//Pixel rate in Hz:
//   pixelClock,
//Color encoding data:
//   burstStart,burstLength,colorClock,phaseAlternating
//Graphical adjustment of aspect ratio:
//   aspect


//=========
//PAL MODES
//=========
//625 lines/frame, 25 frames/s, h 15.625 kHz, v 50 Hz, visible horizontal proportion 0.812


const ModeComposite CompMode::MODEPAL288P(12, 38, 62, 400,  1,  6, 5, 5,  15, 288, 0, 0, 0, 0, 1, 8000000);
//312 line / 50 frame
//288 active lines

//125 ns/pix
//512 pix/line, 64 us/line, 15.625 kHz hSync, 4.75 us hSync
//312 lines/field-progressiveframe, 19.968 ms/field, 50.080 Hz vSync


const ModeComposite CompMode::MODEPAL576I(12, 38, 62, 400,  1,  5, 5, 5,  15, 288, 1, 0, 2, 1, 1, 8000000);
//625 line / 25 frame
//576 active lines

// every field has 1 half-line less (vPreEqHL 6->5) and 4 half-lines (1+2+0+1) are added to vsync part

//125 ns/pix
//512 pix/line, 64 us/line, 15.625 kHz hSync, 4.75 us hSync
//312.5 lines/field, 20 ms/field, 50 Hz vSync


const ModeComposite CompMode::MODEPAL288Pmax(20, 64, 88, 704,  1,  6, 5, 5,  15, 288, 0, 0, 0, 0, 1, 13333333);
//312 line / 50 frame
//288 active lines

//75 ns/pix
//856 pix/line, 64.200 us/line, 15.576324 kHz hSync, 4.8 us hSync
//312 lines/field-progressiveframe, 20.0304 ms/field, 49.924 Hz vSync


const ModeComposite CompMode::MODEPAL288Pmin(2, 5, 5, 52,  1,  6, 5, 5,  15, 288, 0, 0, 0, 0, 1, 1000000);
//312 line / 50 frame
//288 active lines

//1000 ns/pix
//64 pix/line, 64 us/line, 15.625 kHz hSync, 5 us hSync
//312 lines/field-progressiveframe, 19.968 ms/field, 50.080 Hz vSync


const ModeComposite CompMode::MODEPAL576Imin(2, 5, 5, 52,  1,  5, 5, 5,  15, 288, 1, 0, 2, 1, 1, 1000000);
//625 line / 25 frame
//576 active lines

//1000 ns/pix
//64 pix/line, 64 us/line, 15.625 kHz hSync, 5 us hSync
//312.5 lines/field, 20 ms/field, 50 Hz vSync


const ModeComposite CompMode::MODEPAL576Idiv3(12, 38, 62, 400,  1,  5, 5, 5,  15, 288, 1, 0, 2, 1, 3, 8000000);
//625 line / 25 frame
//576 active lines

//125 ns/pix
//512 pix/line, 64 us/line, 15.625 kHz hSync, 4.75 us hSync
//312.5 lines/field, 20 ms/field, 50 Hz vSync



//==========
//NTSC MODES
//==========
//525 lines/frame, 30 frames/s, h 15.750 kHz, v 60 Hz, visible horizontal proportion 0.812


const ModeComposite CompMode::MODENTSC240P(12, 38, 58, 400,  1,  6, 6, 6,  12, 240, 0, 0, 0, 0, 1, 8000000);
//262 line / 60 frame
//240 active lines

//125 ns/pix
//508 pix/line, 63.5 us/line, 15.74803 kHz hSync, 4.75 us hSync
//262 lines/field-progressiveframe, 16.637 ms/field, 60.107 Hz vSync


const ModeComposite CompMode::MODENTSC480I(12, 38, 58, 400,  1,  6, 6, 6,  12, 240, 0, 0, 1, 1, 1, 8000000);
//525 line / 30 frame
//480 active lines

//125 ns/pix
//508 pix/line, 63.5 us/line, 15.74803 kHz hSync, 4.75 us hSync
//262.5 lines/field, 16.668750 ms/field, 59.9925 Hz vSync


const ModeComposite CompMode::MODENTSC240Pmax(20, 64, 76, 688,  1,  6, 6, 6,  12, 240, 0, 0, 0, 0, 1, 13333333);
//262 line / 60 frame
//240 active lines

//75 ns/pix
//848 pix/line, 63.6 us/line, 15.72327 kHz hSync, 4.8 us hSync
//262 lines/field-progressiveframe, 16.6632 ms/field, 60.0125 Hz vSync
