#pragma once

/*
    Author: Martin-Laclaustra 2020
    License: 
    Creative Commons Attribution ShareAlike 4.0
    https://creativecommons.org/licenses/by-sa/4.0/
*/

// integersinaprox
// input: positive interger in 1/256 circumference units
// output: integer between -127 and 127

static inline int8_t integersinaprox(int32_t x) {
    x &= 0xff;
    x -= 128;
    x = x * (abs(x) - 128);
    x += 128<<5;
    x >>= 5;
    x -= 128;
    //extra precision
    x = ((x + 128)<<13) + 14 * x * (abs(x) - 128);
    //bound limits: *127/128
    x *= 127;
    //bound limits division (7) + extra precision rescaling (13)
    x >>= (7+13);
    x -= 127;
    return x;
}


// integeratan2aprox
// input: interger x and y coordinates
// output: positive integer between 0 and 2^16, representing the full circumference

#define FULLCIRCLESIZE 0x00010000L
#define OCTANTSIZE 0x00002000L

static inline uint32_t integeratan2aprox(const int32_t y, const int32_t x) {
    if(y==0 && x==0) return 0;
    bool swapped = abs(y) > abs(x);
    int32_t slope = (swapped ? (x*OCTANTSIZE)/y : (y*OCTANTSIZE)/x);
    uint8_t octant = (y < 0) * 0b111; // bit2 of octant variable
    octant ^= (x < 0) * 0b11; // bit1 of octant variable
    octant ^= swapped; // bit0 of octant variable
    switch(octant) {
      case 0 : return 0*2*OCTANTSIZE + slope;
      case 1 : return 1*2*OCTANTSIZE - slope;
      case 2 : return 1*2*OCTANTSIZE - slope;
      case 3 : return 2*2*OCTANTSIZE + slope;
      case 4 : return 2*2*OCTANTSIZE + slope;
      case 5 : return 3*2*OCTANTSIZE - slope;
      case 6 : return 3*2*OCTANTSIZE - slope;
      case 7 : return 4*2*OCTANTSIZE + slope;
    }
    return 0; // to avoid false return-type (control reaches end of non-void function) compilation error
}
