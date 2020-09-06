/*
 * SmartMatrix Library - Background Layer Class
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

#ifndef _LAYER_BACKGROUND_GFX_H_
#define _LAYER_BACKGROUND_GFX_H_

#include "Layer.h"
#include "MatrixCommon.h"
#include "MatrixFontCommon.h"

#define SM_BACKGROUND_GFX_OPTIONS_NONE     0

template <typename RGB, unsigned int optionFlags>
class SMLayerBackgroundGFX : public SM_Layer {
    public:
        /* RGB specific methods */
        SMLayerBackgroundGFX(RGB * buffer, uint16_t width, uint16_t height, color_chan_t * colorCorrectionLUT);
        SMLayerBackgroundGFX(uint16_t width, uint16_t height);
        void begin(void);
        void frameRefreshCallback();
        void fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[], int brightnessShifts = 0);
        void fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[], int brightnessShifts = 0);
        void copyRefreshToDrawing(void);

        // could make this generic if moving the buffer copy code to a new function
        void swapBuffers(bool copy = true);

        /* RGB Specific Core Drawing Methods */
        void drawPixel(int16_t x, int16_t y, const RGB& color);

        /* RGB Specific Adafruit_GFX methods  */

        /* RGB Specific SmartMatrix Library 3.0 Backwards Compatibility */
        void drawFastHLine(int16_t x0, int16_t x1, int16_t y, const RGB& color);
        void drawFastVLine(int16_t x, int16_t y0, int16_t y1, const RGB& color);
        void fillScreen(const RGB& color);

        /* RGB Specific Raw Buffer Access */
        // reads pixel from drawing buffer, not refresh buffer
        const RGB readPixel(int16_t x, int16_t y);
        RGB *backBuffer(void);
        void setBackBuffer(RGB *newBuffer);
        RGB *getRealBackBuffer();

        /* Shared */
        void setBrightnessShifts(int numShifts);
        int getRequestedBrightnessShifts();
        bool isLayerChanged();
        bool isSwapPending();
        void setBrightness(uint8_t brightness);
        void enableColorCorrection(bool enabled);

        /* Shared SmartMatrix Library 3.0 Backwards Compatibility */
        // right category?
        void drawChar(int16_t x, int16_t y, const RGB& charColor, char character);
        void drawString(int16_t x, int16_t y, const RGB& charColor, const char text[]);
        void drawString(int16_t x, int16_t y, const RGB& charColor, const RGB& backColor, const char text[]);
        void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, const RGB& bitmapColor, const uint8_t *bitmap);
        void setFont(fontChoices newFont);

        /* Replaced by Adafruit_GFX */
        void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
        void drawCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& color);
        void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& outlineColor, const RGB& fillColor);
        void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& color);
        void drawEllipse(int16_t x0, int16_t y0, uint16_t radiusX, uint16_t radiusY, const RGB& color);
        void drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& color);
        void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& fillColor);
        void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3,
            const RGB& outlineColor, const RGB& fillColor);
        void drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
        void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
        void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& outlineColor, const RGB& fillColor);
        void drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const RGB& outlineColor);
        void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const RGB& fillColor);
        void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius,
            const RGB& outlineColor, const RGB& fillColor);

    private:
        // todo: move somewhere else
        static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

        // drawing functions not meant for user
        void drawHardwareHLine(uint16_t x0, uint16_t x1, uint16_t y, const RGB& color);
        void drawHardwareVLine(uint16_t x, uint16_t y0, uint16_t y1, const RGB& color);

        bool ccEnabled = true;

        RGB *currentDrawBufferPtr;
        RGB *currentRefreshBufferPtr;

        RGB *backgroundBuffers[2];

        void loadPixelToDrawBuffer(int16_t hwx, int16_t hwy, const RGB& color);
        const RGB readPixelFromDrawBuffer(int16_t hwx, int16_t hwy);
        void getBackgroundRefreshPixel(uint16_t x, uint16_t y, RGB &refreshPixel);
        bool getForegroundRefreshPixel(uint16_t x, uint16_t y, RGB &xyPixel);

        /* Replaced by Adafruit_GFX */
        void bresteepline(int16_t x3, int16_t y3, int16_t x4, int16_t y4, const RGB& color);
        void fillFlatSideTriangleInt(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& color);

        uint8_t backgroundBrightness = 255;
        color_chan_t * backgroundColorCorrectionLUT;
        bitmap_font *font;

        // idealBrightnessShifts is the number of shifts towards MSB the pixel data can handle without overflowing
        int idealBrightnessShifts = 0;
        // pendingIdealBrightnessShifts keeps track of the data queued up with swapBuffers()
        int pendingIdealBrightnessShifts = 0;

        // keeping track of drawing buffers
        volatile unsigned char currentDrawBuffer;
        volatile unsigned char currentRefreshBuffer;
        volatile bool swapPending;
        void handleBufferSwap(void);
};

#include "Layer_BackgroundGFX_Impl.h"

#endif