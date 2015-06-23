#include "Layer_Foreground.h"

#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 32

const unsigned char foregroundDrawBuffer = 0;
const unsigned char foregroundRefreshBuffer = 1;

SMLayerForeground::SMLayerForeground(void) {

}

void SMLayerForeground::frameRefreshCallback(void) {

}

// returns true and copies color to xyPixel if pixel is opaque, returns false if not
bool SMLayerForeground::getForegroundPixel(uint8_t hardwareX, uint8_t hardwareY, rgb24 &xyPixel) {
    uint8_t localScreenX, localScreenY;

    // convert hardware x/y to the pixel in the local screen
    switch( screenConfig.rotation ) {
      case rotation0 :
        localScreenX = hardwareX;
        localScreenY = hardwareY;
        break;
      case rotation180 :
        localScreenX = (MATRIX_WIDTH - 1) - hardwareX;
        localScreenY = (MATRIX_HEIGHT - 1) - hardwareY;
        break;
      case  rotation90 :
        localScreenX = hardwareY;
        localScreenY = (MATRIX_WIDTH - 1) - hardwareX;
        break;
      case  rotation270 :
        localScreenX = (MATRIX_HEIGHT - 1) - hardwareY;
        localScreenY = hardwareX;
        break;
      default:
        // TODO: Should throw an error
        return false;
    };

    uint32_t bitmask = 0x01 << (31 - localScreenX);

    if (foregroundBitmap[foregroundRefreshBuffer][localScreenY][0] & bitmask) {
        copyRgb24(xyPixel, textcolor);
        return true;
    }

    return false;
}

void SMLayerForeground::getRefreshPixel(uint8_t hardwareX, uint8_t hardwareY, rgb48 &xyPixel) {
    rgb24 tempPixel;

    // do once per refresh
    bool bHasCC = ccmode != ccNone;

    if(getForegroundPixel(hardwareX, hardwareY, tempPixel)) {
        if(bHasCC) {
            // load foreground pixel with color correction
            xyPixel.red = colorCorrection(tempPixel.red);
            xyPixel.green = colorCorrection(tempPixel.green);
            xyPixel.blue = colorCorrection(tempPixel.blue);
        } else {
            // load foreground pixel without color correction
            xyPixel.red = tempPixel.red << 8;
            xyPixel.green = tempPixel.green << 8;
            xyPixel.blue = tempPixel.blue << 8;
        }
    }
}

void SMLayerForeground::getRefreshPixel(uint8_t hardwareX, uint8_t hardwareY, rgb24 &xyPixel) {
    rgb24 tempPixel;

    // do once per refresh
    bool bHasCC = ccmode != ccNone;

    if(getForegroundPixel(hardwareX, hardwareY, tempPixel)) {
        if(bHasCC) {
            // load foreground pixel with color correction
            xyPixel.red = colorCorrection(tempPixel.red);
            xyPixel.green = colorCorrection(tempPixel.green);
            xyPixel.blue = colorCorrection(tempPixel.blue);
        } else {
            // load foreground pixel without color correction
            xyPixel.red = tempPixel.red;
            xyPixel.green = tempPixel.green;
            xyPixel.blue = tempPixel.blue;
        }
    }
}

void SMLayerForeground::setScrollColor(const rgb24 & newColor) {
    copyRgb24(textcolor, newColor);
}

void SMLayerForeground::setColorCorrection(colorCorrectionModes mode) {
    ccmode = mode;
}
