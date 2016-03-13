/*
 * SmartMatrix Library - Indexed Layer Class
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

#ifndef _LAYER_INDEXED_H_
#define _LAYER_INDEXED_H_

#include "Layer.h"
#include "MatrixCommon.h"

#define SM_INDEXED_OPTIONS_NONE     0

// font
#include "MatrixFontCommon.h"

template <typename RGB, unsigned int optionFlags>
class SMLayerIndexed : public SM_Layer {
    public:
        SMLayerIndexed(uint8_t * bitmap, uint16_t width, uint16_t height);
        void frameRefreshCallback();
        void fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[]);
        void fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[]);


        void enableColorCorrection(bool enabled);

        void setIndexedColor(uint8_t index, const RGB & newColor);
        void fillScreen(uint8_t index);
        // behavior is a little different from SMLayerBackground.swapBuffers() - will always copy, but bool forces waiting to avoid updating the drawing buffer before refresh is updated
        void swapBuffers(bool copy = true);
        void drawPixel(int16_t x, int16_t y, uint8_t index);
        void setFont(fontChoices newFont);
        // todo: handle index (draw transparent)
        void drawChar(int16_t x, int16_t y, uint8_t index, char character);
        void drawString(int16_t x, int16_t y, uint8_t index, const char text []);
        void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t index, uint8_t *bitmap);

    private:
        // todo: move somewhere else
        static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

        template <typename RGB_OUT>
        bool getPixel(uint16_t hardwareX, uint16_t hardwareY, RGB_OUT &xyPixel);

        // bitmap size is 32 rows (supporting maximum dimension of screen height in all rotations), by 32 bits
        // double buffered to prevent flicker while drawing
        uint8_t * indexedBitmap;

        void handleBufferCopy(void);

        RGB color = RGB(0xffff, 0xffff, 0xffff);
        unsigned char currentframe = 0;
        char text[textLayerMaxStringLength];

        unsigned char textlen;
        int scrollcounter = 0;
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

        volatile bool copyPending = false;

        bitmap_font *layerFont = (bitmap_font *) &apple3x5;
};

#include "Layer_Indexed_Impl.h"

#endif

