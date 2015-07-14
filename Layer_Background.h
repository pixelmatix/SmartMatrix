#ifndef _LAYER_BACKGROUND_H_
#define _LAYER_BACKGROUND_H_

#include "Layer.h"
#include "MatrixCommon.h"
#include "MatrixFontCommon.h"

class SMLayerBackground : public SM_Layer {
    public:
        SMLayerBackground(rgb24 * buffer, uint8_t width, uint8_t height);
        void frameRefreshCallback();
        void getRefreshPixel(uint8_t x, uint8_t y, rgb24 &refreshPixel);
        void getRefreshPixel(uint8_t x, uint8_t y, rgb48 &refreshPixel);

        void swapBuffers(bool copy = true);
    #ifdef SMARTMATRIX_TRIPLEBUFFER
        void swapBuffersWithInterpolation_frames(int framesToInterpolate, bool copy = false);
        void swapBuffersWithInterpolation_ms(int interpolationSpan_ms, bool copy = false);
    #endif
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
        const rgb24 readPixel(int16_t x, int16_t y);
        rgb24 *backBuffer(void);
        void setBackBuffer(rgb24 *newBuffer);
        rgb24 *getRealBackBuffer(void);
        void setFont(fontChoices newFont);
        void setBrightness(uint8_t brightness);
        void setColorCorrection(colorCorrectionModes mode);

    private:
        colorCorrectionModes ccmode = cc48;

        rgb24 * backgroundBuffer;

        static color_chan_t backgroundColorCorrection(uint8_t inputcolor);

        rgb24 *getCurrentRefreshRow(uint8_t y);
    #ifdef SMARTMATRIX_TRIPLEBUFFER
        rgb24 *getPreviousRefreshRow(uint8_t y);
        static uint32_t calculateFcInterpCoefficient();
    #endif
    #if COLOR_DEPTH_RGB > 24
        void getBackgroundRefreshPixel(uint8_t x, uint8_t y, rgb48 &refreshPixel);
        bool getForegroundRefreshPixel(uint8_t x, uint8_t y, rgb48 &xyPixel);
    #else
        void getBackgroundRefreshPixel(uint8_t x, uint8_t y, rgb24 &refreshPixel);
        bool getForegroundRefreshPixel(uint8_t x, uint8_t y, rgb24 &xyPixel);
    #endif

        // drawing functions not meant for user
        void drawHardwareHLine(uint8_t x0, uint8_t x1, uint8_t y, const rgb24& color);
        void drawHardwareVLine(uint8_t x, uint8_t y0, uint8_t y1, const rgb24& color);
        void bresteepline(int16_t x3, int16_t y3, int16_t x4, int16_t y4, const rgb24& color);
        void fillFlatSideTriangleInt(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const rgb24& color);
        static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

        static uint8_t backgroundBrightness;

        // keeping track of drawing buffers
        static unsigned char currentDrawBuffer;
        static unsigned char currentRefreshBuffer;
    #ifdef SMARTMATRIX_TRIPLEBUFFER    
        static unsigned char previousRefreshBuffer;
    #endif
        static volatile bool swapPending;
        static bool swapWithCopy;
        void handleBufferSwap(void);
};

#endif