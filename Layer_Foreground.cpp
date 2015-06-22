#include "Layer_Foreground.h"


SMLayerForeground::SMLayerForeground(void) {

}

void SMLayerForeground::frameRefreshCallback(void) {

}

void SMLayerForeground::getRefreshPixel(uint8_t x, uint8_t y, rgb24 &refreshPixel) {
    if(x == y)
        copyRgb24(refreshPixel, {0xff, 0xff, 0xff});
}

void SMLayerForeground::getRefreshPixel(uint8_t x, uint8_t y, rgb48 &refreshPixel) {
    rgb48 color48;
    color48.red = textcolor.red << 8;
    color48.green = textcolor.green << 8;
    color48.blue = textcolor.blue << 8;

    if(x == y)
        copyRgb48(refreshPixel, color48);
}

void SMLayerForeground::setScrollColor(const rgb24 & newColor) {
    copyRgb24(textcolor, newColor);
}

void SMLayerForeground::setColorCorrection(colorCorrectionModes mode) {
    ccmode = mode;
}
