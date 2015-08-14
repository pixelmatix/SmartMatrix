#ifndef _MATRIX_COMMON_H_
#define _MATRIX_COMMON_H_

#include <stdint.h>

#ifdef ARDUINO_ARCH_AVR
#include "Arduino.h"
#endif

typedef struct rgb24 {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb24;

typedef struct rgb48 {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
} rgb48;

// replace with class and copy constructor?
void copyRgb24(rgb24 &dst, const rgb24 &src);
void copyRgb48(rgb48 &dst, const rgb48 &src);

#define RGB24_ISEQUAL(a, b)     ((a.red == b.red) && (a.green == b.green) && (a.blue == b.blue))

typedef enum colorCorrectionModes {
    ccNone,
    cc24,
    cc12,
    cc48,
    cc48dither
} colorCorrectionModes;


//#if COLOR_DEPTH_RGB > 24
#define Chan8ToColor( c ) ((c) << 8)
//#else
//#define Chan8ToColor( c ) (c)
//#endif


//#if COLOR_DEPTH_RGB > 24
#define color_chan_t uint16_t
//#else
//#define color_chan_t uint8_t
//#endif

color_chan_t colorCorrection(uint8_t inputcolor);
rgb24 colorCorrectionRgb24(uint8_t r, uint8_t g, uint8_t b);

void calculateBackgroundLUT(color_chan_t * lut, uint8_t backgroundBrightness);

// config
typedef enum rotationDegrees {
    rotation0,
    rotation90,
    rotation180,
    rotation270
} rotationDegrees;

#endif
