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

void copyScreenConfig(screen_config &dst, const screen_config &src) {
    dst.rotation = src.rotation;
    dst.localWidth = src.localWidth;
    dst.localHeight = src.localHeight;
}
