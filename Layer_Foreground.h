#ifndef _LAYER_FOREGROUND_H_
#define _LAYER_FOREGROUND_H_

#include "Arduino.h"

#include "Layer.h"
#include "MatrixCommon.h"

// scroll text
const int textLayerMaxStringLength = 100;

typedef enum ScrollMode {
    wrapForward,
    bounceForward,
    bounceReverse,
    stopped,
    off,
    wrapForwardFromLeft,
} ScrollMode;


// font
#include "MatrixFontCommon.h"
extern const bitmap_font apple3x5;
extern const bitmap_font apple5x7;
extern const bitmap_font apple6x10;
extern const bitmap_font apple8x13;
extern const bitmap_font gohufont6x11;
extern const bitmap_font gohufont6x11b;

typedef enum fontChoices {
    font3x5,
    font5x7,
    font6x10,
    font8x13,
    gohufont11,
    gohufont11b
} fontChoices;

class SMLayerForeground : public SM_Layer {
    public:
        SMLayerForeground();
        void frameRefreshCallback();
        void getRefreshPixel(uint8_t x, uint8_t y, rgb24 &refreshPixel);
        void getRefreshPixel(uint8_t x, uint8_t y, rgb48 &refreshPixel);

        void setScrollColor(const rgb24 & newColor);
        colorCorrectionModes ccmode = cc48;
        void setColorCorrection(colorCorrectionModes mode);

        //bitmap size is 32 rows (supporting maximum dimension of screen height in all rotations), by 32 bits
        // double buffered to prevent flicker while drawing
        uint32_t foregroundBitmap[2][32][32 / 32];

    private:
        rgb24 textcolor = {0xff, 0xff, 0xff};
        bool getForegroundPixel(uint8_t hardwareX, uint8_t hardwareY, rgb24 &xyPixel);

};

#endif

