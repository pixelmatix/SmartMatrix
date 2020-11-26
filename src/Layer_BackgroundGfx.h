/*
 * SmartMatrix Library - Background Layer Class
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

#ifndef _LAYER_BACKGROUND_GFX_H_
#define _LAYER_BACKGROUND_GFX_H_

#include "Layer.h"
#include "MatrixCommon.h"
#include "MatrixFontCommon.h"

// Adafruit_GFX includes
#include "MatrixGfxFontCommon.h"

#define SM_BACKGROUND_GFX_OPTIONS_NONE     0

#define SM_BACKGROUND_GFX_BACKWARDS_COMPATIBILITY
//#define SM_BACKGROUND_GFX_OLD_DRAWING_FUNCTIONS

template <typename RGB, unsigned int optionFlags>
class SMLayerBackgroundGFX : public SM_Layer, public Adafruit_GFX {
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

        /* RGB Specific Adafruit_GFX methods */
        void drawPixel(int16_t x, int16_t y, uint16_t color);

        /* RGB Specific SmartMatrix Library 3.0 Backwards Compatibility */
#ifdef SM_BACKGROUND_GFX_BACKWARDS_COMPATIBILITY
        void drawFastHLine(int16_t x0, int16_t x1, int16_t y, const RGB& color);
        void drawFastVLine(int16_t x, int16_t y0, int16_t y1, const RGB& color);
        void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
        void fillScreen(const RGB& color);
#endif

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
        void setRotation(rotationDegrees newrotation);

        /* Shared SmartMatrix Library 3.0 Backwards Compatibility */
#ifdef SM_BACKGROUND_GFX_BACKWARDS_COMPATIBILITY
        void drawChar(int16_t x, int16_t y, const RGB& charColor, char character);
        void drawString(int16_t x, int16_t y, const RGB& charColor, const char text[]);
        void drawString(int16_t x, int16_t y, const RGB& charColor, const RGB& backColor, const char text[]);
        void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, const RGB& bitmapColor, const uint8_t *bitmap);
        void setFont(fontChoices newFont);
#endif

        /* Replaced by Adafruit_GFX */
        void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
        void drawCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& color);
        void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& color);
        void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& outlineColor, const RGB& fillColor);
        void drawEllipse(int16_t x0, int16_t y0, uint16_t radiusX, uint16_t radiusY, const RGB& color);
        void drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& color);
        void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& fillColor);
        void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3,
            const RGB& outlineColor, const RGB& fillColor);
        void drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
        void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& outlineColor, const RGB& fillColor);
        void drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const RGB& outlineColor);
        void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const RGB& fillColor);
        void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius,
            const RGB& outlineColor, const RGB& fillColor);

        /* Adafruit_GFX methods */
        void setRotation(uint8_t x);
        inline uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return rgb16(red, green, blue).rgb; }
        using Adafruit_GFX::drawLine;
        using Adafruit_GFX::drawTriangle;
        using Adafruit_GFX::drawCircle;
        using Adafruit_GFX::fillCircle;
        using Adafruit_GFX::fillTriangle;
        using Adafruit_GFX::setFont;
        using Adafruit_GFX::drawChar;
        using Adafruit_GFX::fillScreen;
        using Adafruit_GFX::drawFastVLine;
        using Adafruit_GFX::drawFastHLine;

    private:
        // Note we'd use a function template for the public functions but are keeping them fixed with rgb24/rgb48 parameters for backwards compatibility
        template <typename RGB_OUT>
        void fillRefreshRowTemplated(uint16_t hardwareY, RGB_OUT refreshRow[], int brightnessShifts);

        // drawing functions not meant for user
        void drawHardwareHLine(uint16_t x0, uint16_t x1, uint16_t y, const RGB& color);
        void drawHardwareVLine(uint16_t x, uint16_t y0, uint16_t y1, const RGB& color);

        bool ccEnabled = true;

        RGB *currentDrawBufferPtr;
        RGB *currentRefreshBufferPtr;

        RGB *backgroundBuffers[2];

        RGB passThruColor;
        bool passThruColorFlag = false;

        void loadPixelToDrawBuffer(int16_t hwx, int16_t hwy, const RGB& color);
        const RGB readPixelFromDrawBuffer(int16_t hwx, int16_t hwy);
        void getBackgroundRefreshPixel(uint16_t x, uint16_t y, RGB &refreshPixel);
        bool getForegroundRefreshPixel(uint16_t x, uint16_t y, RGB &xyPixel);

        /* Replaced by Adafruit_GFX */
#ifdef SM_BACKGROUND_GFX_OLD_DRAWING_FUNCTIONS
        void bresteepline(int16_t x3, int16_t y3, int16_t x4, int16_t y4, const RGB& color);
        void fillFlatSideTriangleInt(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& color);
        bitmap_font *font;
#endif

        uint8_t backgroundBrightness = 255;
        color_chan_t * backgroundColorCorrectionLUT;

        int16_t layerXOffset = 0;
        int16_t layerYOffset = 0;

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

#include "Layer_BackgroundGfx_Impl.h"

#endif