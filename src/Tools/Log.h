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
#include "Arduino.h"


#define DEBUG_PRINTLN(a) Serial.println(a)
#define DEBUG_PRINT(a) Serial.print(a)
#define DEBUG_PRINTLNF(a, f) Serial.println(a, f)
#define DEBUG_PRINTF(a, f) Serial.print(a, f)
/*
#define DEBUG_PRINTLN(a) ;
#define DEBUG_PRINT(a) ;
#define DEBUG_PRINTLNF(a, f) ;
#define DEBUG_PRINTF(a, f) ;
*/
#define ERROR(a) {Serial.println((a)); delay(3000); throw 0;};
