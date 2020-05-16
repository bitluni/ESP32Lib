/*
	Author: bitluni 2019 Modified by Martin-Laclaustra 2020
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://github.com/bitluni
*/
#include "VGAMode.h"

//Mode arguments order:
//==============================
//Horizontal pixels (in order):
//   hFront,hSync,hBack,hRes,
//Vertical lines (in order):
//   vFront,vSync,vBack,vRes,
//Vertical resolution reduction:
//   vDiv,
//Pixel rate in Hz:
//   pixelClock,
//Sync signals (1 if negative):
//   hSyncPolarity,vSyncPolarity,
//Graphical adjustment of aspect ratio:
//   aspect

const Mode VGAMode::MODE320x480(8, 48, 24, 320, 11, 2, 31, 480, 1, 12587500, 1, 1);
const Mode VGAMode::MODE320x240(8, 48, 24, 320, 11, 2, 31, 480, 2, 12587500, 1, 1);
const Mode VGAMode::MODE320x400(8, 48, 24, 320, 12, 2, 35, 400, 1, 12587500, 1, 0);
const Mode VGAMode::MODE320x200(8, 48, 24, 320, 12, 2, 35, 400, 2, 12587500, 1, 0);
const Mode VGAMode::MODE360x400(8, 54, 28, 360, 11, 2, 32, 400, 1, 14161000, 1, 0);
const Mode VGAMode::MODE360x200(8, 54, 28, 360, 11, 2, 32, 400, 2, 14161000, 1, 0);
const Mode VGAMode::MODE360x350(8, 54, 28, 360, 11, 2, 32, 350, 1, 14161000, 1, 1);
const Mode VGAMode::MODE360x175 (8, 54, 28, 360, 11, 2, 32, 350, 2, 14161000, 1, 1);

const Mode VGAMode::MODE320x350 (8, 48, 24, 320, 37, 2, 60, 350, 1, 12587500, 0, 1);
const Mode VGAMode::MODE320x175(8, 48, 24, 320, 37, 2, 60, 350, 2, 12587500, 0, 1);

const Mode VGAMode::MODE400x300(12, 36, 64, 400, 1, 2, 22, 600, 2, 18000000, 0, 0);
const Mode VGAMode::MODE400x150(12, 36, 64, 400, 1, 2, 22, 600, 4, 18000000, 0, 0);
const Mode VGAMode::MODE400x100(12, 36, 64, 400, 1, 2, 22, 600, 6, 18000000, 0, 0);
const Mode VGAMode::MODE200x150(6, 18, 32, 200, 1, 2, 22, 600, 4, 9000000, 0, 0);
//const Mode VGAMode::MODE200x150(10, 32, 22, 200, 1, 4, 23, 600, 4, 10000000, 0, 0);	//60Hz version

//500 pixels horizontal it's based on 640x480
const Mode VGAMode::MODE500x480(12, 76, 38, 500, 11, 2, 31, 480, 1, 19667968, 1, 1);
const Mode VGAMode::MODE500x240(12, 76, 38, 500, 11, 2, 31, 480, 2, 19667968, 1, 1);

//base modes for custom mode calculations
const Mode VGAMode::MODE1280x1024(48, 112, 248, 1280, 1, 3, 38, 1024, 1, 108000000, 0, 0);
const Mode VGAMode::MODE1280x960(80, 136, 216, 1280, 1, 3, 30, 960, 1, 101200000, 1, 0);
const Mode VGAMode::MODE1280x800(64, 136, 200, 1280, 1, 3, 24, 800, 1, 83640000, 1, 0);
const Mode VGAMode::MODE1024x768(24, 136, 160, 1024, 3, 6, 29, 768, 1, 65000000, 1, 1);
const Mode VGAMode::MODE800x600(24, 72, 128, 800, 1, 2, 22, 600, 1, 36000000, 0, 0);
const Mode VGAMode::MODE720x400(16, 108, 56, 720, 11, 2, 32, 400, 1, 28322000, 1, 0);
const Mode VGAMode::MODE720x350(16, 108, 56, 720, 11, 2, 32, 350, 1, 28322000, 1, 1);
const Mode VGAMode::MODE640x480(16, 96, 48, 640, 11, 2, 31, 480, 1, 25175000, 1, 1);
const Mode VGAMode::MODE640x400(16, 96, 48, 640, 12, 2, 35, 400, 1, 25175000, 1, 0);
const Mode VGAMode::MODE640x350(16, 96, 48, 640, 37, 2, 60, 350, 1, 25175000, 0, 1);

