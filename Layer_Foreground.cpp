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
    if(x == y)
        copyRgb48(refreshPixel, {0xffff, 0xffff, 0xffff});
}
