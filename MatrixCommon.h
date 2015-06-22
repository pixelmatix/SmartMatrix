#ifndef _MATRIX_COMMON_H_
#define _MATRIX_COMMON_H_

#include <stdint.h>

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
void copyRgb24(rgb24 & dst, const rgb24 &src);
void copyRgb48(rgb48 & dst, const rgb48 &src);

#define RGB24_ISEQUAL(a, b)     ((a.red == b.red) && (a.green == b.green) && (a.blue == b.blue))


#endif
