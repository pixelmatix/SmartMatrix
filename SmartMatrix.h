/*
 * SmartMatrix Library - Main Header File for SmartMatrix Class
 *
 * Copyright (c) 2014 Louis Beaudoin (Pixelmatix)
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

#ifndef SmartMatrix_h
#define SmartMatrix_h

#include "Arduino.h"
#include <stdint.h>

// include one of the MatrixHardware_*.h files here:
//#include "MatrixHardware_KitV1_32x32.h"
//#include "MatrixHardware_KitV1_16x32.h"
#include "MatrixHardware_KitV1_generic.h"


// fallbacks for DRAWING_WIDTH and DRAWING_HEIGHT
#ifndef DRAWING_WIDTH
  #define DRAWING_WIDTH  MATRIX_WIDTH
#endif

#ifndef DRAWING_HEIGHT
  #define DRAWING_HEIGHT MATRIX_HEIGHT
#endif

#ifndef DISABLE_FOREGROUND_FUNCTIONS
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
#endif


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


#if defined(SMARTMATRIX_COLSPACE_RGB565)
struct rgb24 {
    uint16_t red   :5;
    uint16_t green :6;
    uint16_t blue  :5;

    inline rgb24() __attribute__((always_inline)) {
    }

    inline rgb24( uint8_t r, uint8_t g, uint8_t b)  __attribute__((always_inline))
      : red(r >> 3), green(g >> 2), blue(b >> 3)
    {
    }

    inline rgb24& operator= (const uint8_t raw[3])  __attribute__((always_inline)) {
      red = raw[0] >> 3;
      green = raw[1] >> 2;
      blue = raw[2] >> 3;
      return *this;
    }
};

// for (i = 0 ; i < bits; i++) { arr[i] = int(i * 255 / ( ( 1 << bits ) - 1 )); }
const uint8_t colspace_scale6[] =
  { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80,
    85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 129, 133, 137, 141, 145, 149,
    153, 157, 161, 165, 170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 214,
    218, 222, 226, 230, 234, 238, 242, 246, 250, 255
  };
const uint8_t colspace_scale5[] =
  { 0, 8, 16, 24, 32, 41, 49, 57, 65, 74, 82, 90, 98, 106, 115, 123, 131, 139, 148,
    156, 164, 172, 180, 189, 197, 205, 213, 222, 230, 238, 246, 255
  };

// color
struct rgb888 {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    inline rgb888() __attribute__((always_inline)) {
    }

    inline rgb888( uint8_t r, uint8_t g, uint8_t b)  __attribute__((always_inline))
      : red(r), green(g), blue(b)
    {
    }

    inline rgb888( const rgb24& col)  __attribute__((always_inline))
      : red( colspace_scale5[ col.red ]), green( colspace_scale6[ col.green ] ), blue( colspace_scale5[ col.blue ] )
    {
    }
};

#elif defined(SMARTMATRIX_COLSPACE_RGB332)
struct rgb24 {
    uint8_t red   :3;
    uint8_t green :3;
    uint8_t blue  :2;

    inline rgb24() __attribute__((always_inline)) {
    }

    inline rgb24( uint8_t r, uint8_t g, uint8_t b)  __attribute__((always_inline))
      : red(r >> 5), green(g >> 5), blue(b >> 6)
    {
    }

    inline rgb24& operator= (const uint8_t raw[3])  __attribute__((always_inline)) {
      red = raw[0] >> 5;
      green = raw[1] >> 5;
      blue = raw[2] >> 6;
      return *this;
    }
};

// for (i = 0 ; i < bits; i++) { arr[i] = int(i * 255 / ( ( 1 << bits ) - 1 )); }
const uint8_t colspace_scale3[] = { 0, 36, 72, 109, 145, 182, 218, 255 };
const uint8_t colspace_scale2[] = { 0, 85, 170, 255 };

// color
struct rgb888 {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    inline rgb888() __attribute__((always_inline)) {
    }

    inline rgb888( uint8_t r, uint8_t g, uint8_t b)  __attribute__((always_inline))
      : red(r), green(g), blue(b)
    {
    }

    inline rgb888( const rgb24& col)  __attribute__((always_inline))
      : red( colspace_scale3[ col.red ]), green( colspace_scale3[ col.green ] ), blue( colspace_scale2[ col.blue ] )
    {
    }
};



#else  // default colour space RGB888
typedef struct rgb24 {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb24;

#define rgb888 rgb24

#endif



#if COLOR_DEPTH_RGB > 24
#define color_chan_t uint16_t
#else
#define color_chan_t uint8_t
#endif

typedef enum colorCorrectionModes {
    ccNone,
    cc24,
    cc12,
    cc48
} colorCorrectionModes;

#define RGB24_ISEQUAL(a, b)     ((a.red == b.red) && (a.green == b.green) && (a.blue == b.blue))


// config
typedef enum rotationDegrees {
    rotation0,
    rotation90,
    rotation180,
    rotation270
} rotationDegrees;

typedef struct screen_config {
    rotationDegrees rotation;
    uint16_t localWidth;
    uint16_t localHeight;
} screen_config;

#define SMART_MATRIX_CAN_TRIPLE_BUFFER 1

class SmartMatrix {
public:
    SmartMatrix(void);
    void begin(void);

    // drawing functions
    void swapBuffers(bool copy = true);
    void drawPixel(int16_t x, int16_t y, const rgb24& color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color);
    void drawFastVLine(int16_t x, int16_t y0, int16_t y1, const rgb24& color);
    void drawFastHLine(int16_t x0, int16_t x1, int16_t y, const rgb24& color);
    void drawCircle(int16_t x0, int16_t y0, uint16_t radius, const rgb24& color);
    void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const rgb24& outlineColor, const rgb24& fillColor);
    void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const rgb24& color);
    void drawEllipse(int16_t x0, int16_t y0, uint16_t radiusX, uint16_t radiusY, const rgb24& color);
    void drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const rgb24& color);
    void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const rgb24& fillColor);
    void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3,
      const rgb24& outlineColor, const rgb24& fillColor);
    void drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color);
    void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color);
    void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& outlineColor, const rgb24& fillColor);
    void drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const rgb24& outlineColor);
    void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const rgb24& fillColor);
    void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius,
      const rgb24& outlineColor, const rgb24& fillColor);
    void fillScreen(const rgb24& color);
    void drawChar(int16_t x, int16_t y, const rgb24& charColor, char character);
    void drawString(int16_t x, int16_t y, const rgb24& charColor, const char text[]);
    void drawString(int16_t x, int16_t y, const rgb24& charColor, const rgb24& backColor, const char text[]);
    void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, const rgb24& bitmapColor, const uint8_t *bitmap);
    const rgb24 readPixel(int16_t x, int16_t y) const;
    rgb24 *backBuffer(void);
    void setBackBuffer(rgb24 *newBuffer);
    rgb24 *getRealBackBuffer(void);

#ifndef DISABLE_FOREGROUND_FUNCTIONS
    // scroll text
    void scrollText(const char inputtext[], int numScrolls);
    void setScrollMode(ScrollMode mode);
    void setScrollSpeed(unsigned char pixels_per_second);
    void setScrollFont(fontChoices newFont);
    void setScrollColor(const rgb24 & newColor);
#define setScrollOffsetFromEdge setScrollOffsetFromTop // backwards compatibility
    void setScrollOffsetFromTop(int offset);
    void setScrollStartOffsetFromLeft(int offset);
    void updateScrollText(const char inputtext[]);
    void stopScrollText(void);
    int getScrollStatus(void) const;

    // foreground drawing
    void clearForeground(void);
    void setForegroundFont(fontChoices newFont);
    void drawForegroundPixel(int16_t x, int16_t y, bool opaque = true);
    void drawForegroundChar(int16_t x, int16_t y, char character, bool opaque = true);
    void drawForegroundString(int16_t x, int16_t y, const char text [], bool opaque = true);
    void drawForegroundMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *bitmap, bool opaque = true);
    void displayForegroundDrawing(bool waitUntilComplete = true);
#endif

    // configuration
    void setRotation(rotationDegrees rotation);
    uint16_t getScreenWidth(void) const;
    uint16_t getScreenHeight(void) const;
    void setBrightness(uint8_t brightness);
    void setBackgroundBrightness(uint8_t brightness);
    void setColorCorrection(colorCorrectionModes mode);
    void setFont(fontChoices newFont);

private:
    // enable ISR access to private member variables
    friend void rowCalculationISR(void);
    friend void rowShiftCompleteISR(void);

    // functions called by ISR
    static void matrixCalculations(void);

    // functions for refreshing
    static void convertToHardwareXY(int16_t x, int16_t y, int16_t *hwx, int16_t *hwy);
    static void loadMatrixBuffers(unsigned char currentRow);

    static color_chan_t colorCorrection(uint8_t inputcolor);
    static color_chan_t backgroundColorCorrection(uint8_t inputcolor);

    static void getPixel(uint8_t hardwareX, uint8_t hardwareY, rgb24 *xyPixel);
    static rgb24 *getRefreshRow(uint8_t y);
    static void handleBufferSwap(void);
#ifndef DISABLE_FOREGROUND_FUNCTIONS
    static void handleForegroundDrawingCopy(void);
    static void updateForeground(void);
    static bool getForegroundPixel(uint8_t x, uint8_t y, rgb24 *xyPixel);
    static void redrawForeground(void);
    static void setScrollMinMax(void);
#endif

    // drawing functions not meant for user
    void drawHardwareHLine(uint8_t x0, uint8_t x1, uint8_t y, const rgb24& color);
    void drawHardwareVLine(uint8_t x, uint8_t y0, uint8_t y1, const rgb24& color);
    void bresteepline(int16_t x3, int16_t y3, int16_t x4, int16_t y4, const rgb24& color);
    void fillFlatSideTriangleInt(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const rgb24& color);
    static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

    // configuration helper functions
    static void calculateTimerLut(void);
    static void calculateBackgroundLUT(void);

    // color functions (replace with class and copy constructor?)
    static void copyRgb24(rgb24 & dst, const rgb24 & src);

    // configuration
    static colorCorrectionModes _ccmode;
    static screen_config screenConfig;
    static volatile bool brightnessChange;
    static int dimmingFactor;
    static uint8_t backgroundBrightness;
    static const int dimmingMaximum;

    // keeping track of drawing buffers
    static unsigned char currentDrawBuffer;
    static unsigned char currentRefreshBuffer;
    static volatile bool swapPending;
    static volatile bool foregroundCopyPending;
    static bool swapWithCopy;

    // font
    static bool getBitmapFontPixelAtXY(unsigned char letter, unsigned char x, unsigned char y, const bitmap_font *font);
    const bitmap_font *fontLookup(fontChoices font) const;
    static uint16_t getBitmapFontRowAtXY(unsigned char letter, unsigned char y, const bitmap_font *font);
};

#endif
