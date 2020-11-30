/*
 * SmartMatrix Library - Adafruit_GFX Mono Layer Class
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

#ifndef _LAYER_GFX_MONO_H_
#define _LAYER_GFX_MONO_H_

#include "Layer.h"
#include "MatrixCommon.h"
#include "MatrixFontCommon.h"

// Adafruit_GFX includes
#include "MatrixGfxFontCommon.h"

#define SM_GFX_MONO_OPTIONS_NONE     0

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
class SMLayerGFXMono : public SM_Layer, public Adafruit_GFX {
    public:
        /* RGB specific methods */
        SMLayerGFXMono(uint8_t * bitmap, uint16_t width, uint16_t height, uint16_t layerWidth, uint16_t layerHeight);
        SMLayerGFXMono(uint16_t width, uint16_t height, uint16_t layerWidth, uint16_t layerHeight);
        void begin(void);
        void frameRefreshCallback();
        void fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[], int brightnessShifts = 0);
        void fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[], int brightnessShifts = 0);

        // could make this generic if moving the buffer copy code to a new function
        void swapBuffers(bool copy = true);

        void setIndexedColor(uint8_t index, const RGB_API & newColor);
        void enableColorCorrection(bool enabled);
        void setRotation(rotationDegrees newrotation);

        /* RGB Specific Core Drawing Methods */
        void drawPixel(int16_t x, int16_t y, const rgb1 index);
        void fillScreen(const rgb1 index);
        // returns -1 if resize requested was more than memory available
        int resizeLayer(uint16_t width, uint16_t height);

        /* RGB Specific Adafruit_GFX methods */
        void drawPixel(int16_t x, int16_t y, uint16_t color);
        void fillScreen(uint16_t color);

        /* RGB Specific SmartMatrix Library 3.0 Backwards Compatibility */
        void drawPixel(int16_t x, int16_t y, uint8_t index);
        void fillScreen(uint8_t index);

        /* RGB Specific (only because indexed color is set separately) SmartMatrix Library 3.0 Backwards Compatibility */
        void drawChar(int16_t x, int16_t y, uint8_t index, char character);
        void drawString(int16_t x, int16_t y, uint8_t index, const char text []);
        void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t index, uint8_t *bitmap);

        /* Scrolling Text */
        // returns -1 if text is too big to fit in layer
        int start(const char inputtext[], int numScrolls);
        int update(const char inputtext[]);
        void stop(void);
        int getStatus(void) const;
        void setMode(ScrollMode mode);
        void setColor(const RGB_API & newColor);
        void setSpeed(unsigned char pixels_per_second);
        void setOffsetFromTop(int offset);
        void setStartOffsetFromLeft(int offset);

        /* Scrolling Text Helper Function - unique to GFX Layers */
        void getDimensionsOfPrintedString(const char text[], uint16_t *w, uint16_t *h);

        /* RGB Specific Raw Buffer Access */

        /* Shared */

        /* Shared SmartMatrix Library 3.0 Backwards Compatibility */
        void setFont(fontChoices newFont);

        /* Adafruit_GFX methods */
        void setRotation(uint8_t x);
        inline uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return rgb16(red, green, blue).rgb; }
        // we need to tell the compiler that we want to use the methods that are included in Adafruit_GFX, in addition to our overloaded custom versions
        using Adafruit_GFX::drawPixel;
        using Adafruit_GFX::setFont;
        using Adafruit_GFX::drawChar;

        /* Adafruit_GFX wrappers for lazily typed colors (with color defaulting as int) */
        inline void fillScreen(int color) { fillScreen((rgb1)(color > 0)); };
        inline void drawPixel(int16_t x, int16_t y, int color) { drawPixel(x, y, (rgb1)(color > 0)); };

    private:
        /* RGB specific */
        void handleBufferSwap(void);

        // Note we'd use a function template for the public functions but are keeping them fixed with rgb24/rgb48 parameters for backwards compatibility
        template <typename RGB_OUT>
        void fillRefreshRowTemplated(uint16_t hardwareY, RGB_OUT refreshRow[], int brightnessShifts);
        // bitmap size is 32 rows (supporting maximum dimension of screen height in all rotations), by 32 bits
        // double buffered to prevent flicker while drawing
        uint8_t * indexedBitmap;

        size_t bufferSize;

        RGB_API indexedColor[2];
        rgb1 transparentColor = false;
        bool transparencyEnabled = true;

        bool ccEnabled = true;

        int16_t layerXOffset = 0;
        int16_t layerYOffset = 0;

        /* Scrolling Text */
        void updateScrollingText(void);
        bool mirrorHardwareScrollingDirection = false;
        void clearRefreshAndDrawingBuffers(void);        
        
        // returns -1 if text is too big to fit into layer
        int resizeLayerToText(const char inputtext[]);
        void setMinMax(void);
        unsigned char currentframe = 0;
        unsigned char pixelsPerSecond = 30;
        volatile int scrollcounter = 0;
        int16_t fontTopOffset = 1;
        int16_t fontLeftOffset = 1;
        
        ScrollMode scrollmode = bounceForward;
        unsigned char framesperscroll = 2;

        // these variables describe the text bitmap: size, location on the screen, and bounds of where it moves
        unsigned int textWidth;
        int16_t scrollMin, scrollMax;
        int16_t scrollPosition;

        // keeping track of drawing buffers
        volatile unsigned char currentDrawBuffer;
        volatile unsigned char currentRefreshBuffer;
        volatile bool swapPending;
};

#include "Layer_Gfx_Mono_Impl.h"

#endif
