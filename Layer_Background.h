#ifndef _LAYER_BACKGROUND_H_
#define _LAYER_BACKGROUND_H_

#include "Layer.h"
#include "MatrixCommon.h"
#include "MatrixFontCommon.h"

template <typename RGB>
class SMLayerBackground : public SM_Layer {
    public:
        SMLayerBackground(RGB * buffer, uint8_t width, uint8_t height);
        void frameRefreshCallback();
        void getRefreshPixel(uint8_t hardwareX, uint8_t hardwareY, rgb48 &xyPixel);
        void fillRefreshRow(uint8_t hardwareY, rgb48 refreshRow[]);
        void fillRefreshRow(uint8_t hardwareY, rgb24 refreshRow[]);
        void swapBuffers(bool copy = true);
    #ifdef SMARTMATRIX_TRIPLEBUFFER
        void swapBuffersWithInterpolation_frames(int framesToInterpolate, bool copy = false);
        void swapBuffersWithInterpolation_ms(int interpolationSpan_ms, bool copy = false);
    #endif
        void drawPixel(int16_t x, int16_t y, const RGB& color);
        void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
        void drawFastVLine(int16_t x, int16_t y0, int16_t y1, const RGB& color);
        void drawFastHLine(int16_t x0, int16_t x1, int16_t y, const RGB& color);
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
        void fillScreen(const RGB& color);
        void drawChar(int16_t x, int16_t y, const RGB& charColor, char character);
        void drawString(int16_t x, int16_t y, const RGB& charColor, const char text[]);
        void drawString(int16_t x, int16_t y, const RGB& charColor, const RGB& backColor, const char text[]);
        void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, const RGB& bitmapColor, const uint8_t *bitmap);

        // reads pixel from drawing buffer, not refresh buffer
        const RGB readPixel(int16_t x, int16_t y);

        RGB *backBuffer(void);
        void setBackBuffer(RGB *newBuffer);

        RGB *getRealBackBuffer();

        void setFont(fontChoices newFont);
        void setBrightness(uint8_t brightness);
        void enableColorCorrection(bool enabled);

    private:
        bool ccEnabled = sizeof(RGB) <= 3 ? true : false;

        RGB *currentDrawBufferPtr;
        RGB *currentRefreshBufferPtr;

        RGB *backgroundBuffer;

        RGB *getCurrentRefreshRow(uint8_t y);

    #ifdef SMARTMATRIX_TRIPLEBUFFER
        RGB *getPreviousRefreshRow(uint8_t y);
        static uint32_t calculateFcInterpCoefficient();
    #endif
        void getBackgroundRefreshPixel(uint8_t x, uint8_t y, RGB &refreshPixel);
        bool getForegroundRefreshPixel(uint8_t x, uint8_t y, RGB &xyPixel);

        // drawing functions not meant for user
        void drawHardwareHLine(uint8_t x0, uint8_t x1, uint8_t y, const RGB& color);
        void drawHardwareVLine(uint8_t x, uint8_t y0, uint8_t y1, const RGB& color);
        void bresteepline(int16_t x3, int16_t y3, int16_t x4, int16_t y4, const RGB& color);
        void fillFlatSideTriangleInt(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& color);
        // todo: move somewhere else
        static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

        uint8_t backgroundBrightness = 255;

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

#include "Layer_Background_Impl.h"

#endif