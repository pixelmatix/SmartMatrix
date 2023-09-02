/*
 * SmartMatrix Library - Indexed Layer Class
 *
 * Copyright (c) 2020 Louis Beaudoin (Pixelmatix)
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

enum SMIndexedLayerColourDepth : uint32_t
{
    ONEBIT=0,
    TWOBITS=1,
    FOURBITS=3,
    EIGHTBITS=7
};

// font
#include "MatrixFontCommon.h"

template <typename RGB, unsigned int optionFlags>
class SMLayerIndexed : public SM_Layer {
    public:
        SMLayerIndexed(uint8_t * bitmap, uint16_t width, uint16_t height, SMIndexedLayerColourDepth colourDepthBits=ONEBIT);
        SMLayerIndexed(uint16_t width, uint16_t height, SMIndexedLayerColourDepth colourDepthBits=ONEBIT);
        void begin(void);
        void frameRefreshCallback();
        void fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[], int brightnessShifts = 0);
        void fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[], int brightnessShifts = 0);

        void enableColorCorrection(bool enabled);

        void setIndexedColor(uint8_t index, const RGB & newColor);
        void fillScreen(uint8_t index);
        // behavior is a little different from SMLayerBackground.swapBuffers() - will always copy, but bool forces waiting to avoid updating the drawing buffer before refresh is updated
        void swapBuffers(bool copy = true);
        void drawPixel(int16_t x, int16_t y, uint8_t index);
        void setFont(fontChoices newFont);
        // todo: handle index (draw transparent)
        void drawChar(int16_t x, int16_t y, uint8_t index, char character, uint8_t backgroundIndex=0);
        void drawString(int16_t x, int16_t y, uint8_t index, const char text [], uint8_t backgroundIndex=0);
        void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t index, uint8_t *bitmap, uint8_t backgroundIndex=0);
        
    protected:
        // todo: move somewhere else
        static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

        template <typename RGB_OUT>
        bool getPixel(uint16_t hardwareX, uint16_t hardwareY, RGB_OUT &xyPixel);

        // bitmap size is 32 rows (supporting maximum dimension of screen height in all rotations), by 32 bits
        // double buffered to prevent flicker while drawing
        uint8_t * indexedBitmap;

        RGB *colourLookup;

        bool ccEnabled = sizeof(RGB) <= 3 ? true : false;

        // keeping track of drawing buffers
        volatile unsigned char currentDrawBuffer;
        volatile unsigned char currentRefreshBuffer;
        volatile bool swapPending;
        void handleBufferSwap(void);

        bitmap_font *layerFont = (bitmap_font *) &apple3x5;
    private:
        const SMIndexedLayerColourDepth indexColourDepth;
};

#include "Layer_Indexed_Impl.h"

#endif
