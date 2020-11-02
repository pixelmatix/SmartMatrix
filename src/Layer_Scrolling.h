/*
 * SmartMatrix Library - Scrolling Layer Class
 *
 * Copyright (c) 2015 Louis Beaudoin (Pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _LAYER_SCROLLING_H_
#define _LAYER_SCROLLING_H_

#include "Layer.h"
#include "MatrixCommon.h"

// scroll text
const int textLayerMaxStringLength = 1024;

typedef enum ScrollMode {
    wrapForward,
    bounceForward,
    bounceReverse,
    stopped,
    off,
    wrapForwardFromLeft,
} ScrollMode;

#define SM_SCROLLING_OPTIONS_NONE     0

// font
#include "MatrixFontCommon.h"

template <typename RGB, unsigned int optionFlags>
class SMLayerScrolling : public SM_Layer {
    public:
        SMLayerScrolling(uint8_t * bitmap, uint16_t width, uint16_t height);
        void frameRefreshCallback();
        void fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[]);
        void fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[]);

        void setRefreshRate(uint8_t newRefreshRate);

        // size of bitmap is 1 bit per pixel for width*height (no need for double buffering)
        uint8_t * scrollingBitmap;

        void stop(void);
        int getStatus(void) const;
        void start(const char inputtext[], int numScrolls);
        void update(const char inputtext[]);
        void setMode(ScrollMode mode);
        void setColor(const RGB & newColor);
        void setSpeed(unsigned char pixels_per_second);
        void setFont(fontChoices newFont);
        void setOffsetFromTop(int offset);
        void setStartOffsetFromLeft(int offset);
        void enableColorCorrection(bool enabled);

    private:
        void redrawScrollingText(void);
        void setMinMax(void);

        // todo: move somewhere else
        static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);
        void updateScrollingText(void);

        template <typename RGB_OUT>
        bool getPixel(uint16_t hardwareX, uint16_t hardwareY, RGB_OUT &xyPixel);
        bool getPixel(uint16_t hardwareX, uint16_t hardwareY);

        RGB textcolor;
        unsigned char currentframe = 0;
        char text[textLayerMaxStringLength];
        unsigned char pixelsPerSecond = 30;

        unsigned char textlen;
        volatile int scrollcounter = 0;
        const bitmap_font *scrollFont = &apple5x7;

        int fontTopOffset = 1;
        int fontLeftOffset = 1;
        bool majorScrollFontChange = false;

        bool ccEnabled = sizeof(RGB) <= 3 ? true : false;
        ScrollMode scrollmode = bounceForward;
        unsigned char framesperscroll = 4;

        // these variables describe the text bitmap: size, location on the screen, and bounds of where it moves
        unsigned int textWidth;
        int scrollMin, scrollMax;
        int scrollPosition;
};

#include "Layer_Scrolling_Impl.h"

#endif

