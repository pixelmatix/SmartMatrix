#include "MatrixCommon.h"

void copyRgb24(rgb24 & dst, const rgb24 &src) {
    dst.red = src.red;
    dst.green = src.green;
    dst.blue = src.blue;
}

void copyRgb48(rgb48 & dst, const rgb48 &src) {
    dst.red = src.red;
    dst.green = src.green;
    dst.blue = src.blue;
}
