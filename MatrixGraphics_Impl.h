/*
 * SmartMatrix Library - Methods for interacting with background layer
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

#include <stdlib.h>

template <typename RGB>
void SmartMatrix3<RGB>::drawPixel(int16_t x, int16_t y, const RGB& color) {
    if(backgroundLayer)
        backgroundLayer->drawPixel(x,y,color);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawFastHLine(int16_t x0, int16_t x1, int16_t y, const RGB& color) {
    if(backgroundLayer)
        backgroundLayer->drawFastHLine(x0, x1, y, color);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawFastVLine(int16_t x, int16_t y0, int16_t y1, const RGB& color) {
    if(backgroundLayer)
        backgroundLayer->drawFastVLine(x,y0,y1,color);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const RGB& color) {
    if(backgroundLayer)
        backgroundLayer->drawLine(x1,y1,x2,y2,color);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& color) {
    if(backgroundLayer)
        backgroundLayer->drawCircle(x0,y0,radius,color);
}

template <typename RGB>
void SmartMatrix3<RGB>::fillCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& outlineColor, const RGB& fillColor) {
    if(backgroundLayer)
        backgroundLayer->fillCircle(x0,y0,radius,outlineColor,fillColor);
}

template <typename RGB>
void SmartMatrix3<RGB>::fillCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& fillColor) {
    if(backgroundLayer)
        backgroundLayer->fillCircle(x0, y0, radius,fillColor);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawEllipse(int16_t x0, int16_t y0, uint16_t radiusX, uint16_t radiusY, const RGB& color) {
    if(backgroundLayer)
        backgroundLayer->drawEllipse(x0, y0, radiusX, radiusY, color);
}

template <typename RGB>
void SmartMatrix3<RGB>::fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
  uint16_t radius, const RGB& fillColor) {
    if(backgroundLayer)
        backgroundLayer->fillRoundRectangle(x0, y0, x1, y1, radius, fillColor);
}

template <typename RGB>
void SmartMatrix3<RGB>::fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
  uint16_t radius, const RGB& outlineColor, const RGB& fillColor) {
    if(backgroundLayer)
        backgroundLayer->fillRoundRectangle(x0, y0, x1, y1, radius, outlineColor, fillColor);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
  uint16_t radius, const RGB& outlineColor) {
    if(backgroundLayer)
        backgroundLayer->drawRoundRectangle(x0, y0, x1, y1, radius, outlineColor);
}

template <typename RGB>
void SmartMatrix3<RGB>::fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& fillColor) {
    if(backgroundLayer)
        backgroundLayer->fillTriangle(x1, y1, x2, y2, x3, y3, fillColor);
}

template <typename RGB>
void SmartMatrix3<RGB>::fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3,
  const RGB& outlineColor, const RGB& fillColor) {
    if(backgroundLayer)
        backgroundLayer->fillTriangle(x1, y1, x2, y2, x3, y3, outlineColor, fillColor);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& color) {
    if(backgroundLayer)
        backgroundLayer->drawTriangle(x1, y1, x2, y2, x3, y3, color);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color) {
    if(backgroundLayer)
        backgroundLayer->drawRectangle(x0, y0, x1, y1, color);
}

template <typename RGB>
void SmartMatrix3<RGB>::fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color) {
    if(backgroundLayer)
        backgroundLayer->fillRectangle(x0, y0, x1, y1, color);
}

template <typename RGB>
void SmartMatrix3<RGB>::fillScreen(const RGB& color) {
    if(backgroundLayer)
        backgroundLayer->fillScreen(color);
}

template <typename RGB>
void SmartMatrix3<RGB>::fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& outlineColor, const RGB& fillColor) {
    if(backgroundLayer)
        backgroundLayer->fillRectangle(x0, y0, x1, y1, outlineColor, fillColor);
}

template <typename RGB>
void SmartMatrix3<RGB>::setFont(fontChoices newFont) {
    if(backgroundLayer)
        backgroundLayer->setFont(newFont);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawChar(int16_t x, int16_t y, const RGB& charColor, char character) {
    if(backgroundLayer)
        backgroundLayer->drawChar(x,y,charColor,character);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawString(int16_t x, int16_t y, const RGB& charColor, const char text[]) {
    if(backgroundLayer)
        backgroundLayer->drawString(x,y,charColor,text);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawString(int16_t x, int16_t y, const RGB& charColor, const RGB& backColor, const char text[]) {
    if(backgroundLayer)
        backgroundLayer->drawString(x,y,charColor,backColor,text);
}

template <typename RGB>
void SmartMatrix3<RGB>::drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height,
  const RGB& bitmapColor, const uint8_t *bitmap) {
    if(backgroundLayer)
        backgroundLayer->drawMonoBitmap(x,y,width,height, bitmapColor, bitmap);
}

template <typename RGB>
void SmartMatrix3<RGB>::swapBuffers(bool copy) {
    if(backgroundLayer)
        backgroundLayer->swapBuffers(copy);
}

#ifdef SMARTMATRIX_TRIPLEBUFFER
template <typename RGB>
void SmartMatrix3<RGB>::swapBuffersWithInterpolation_frames(int framesToInterpolate, bool copy) {
    if(backgroundLayer)
        backgroundLayer->swapBuffersWithInterpolation_frames(framesToInterpolate, copy);
}

template <typename RGB>
void SmartMatrix3<RGB>::swapBuffersWithInterpolation_ms(int interpolationSpan_ms, bool copy) {
    if(backgroundLayer)
        backgroundLayer->swapBuffersWithInterpolation_ms(interpolationSpan_ms, copy);
}
#endif

template <typename RGB>
RGB SmartMatrix3<RGB>::readPixel(int16_t x, int16_t y) {
    if(backgroundLayer)
        return backgroundLayer->readPixel(x, y);
    return {0,0,0};
}

template <typename RGB>
RGB *SmartMatrix3<RGB>::backBuffer(void) {
    if(backgroundLayer)
        return backgroundLayer->backBuffer();
    return 0;
}

template <typename RGB>
void SmartMatrix3<RGB>::setBackBuffer(RGB *newBuffer) {
    if(backgroundLayer)
        backgroundLayer->setBackBuffer(newBuffer);
}

template <typename RGB>
RGB *SmartMatrix3<RGB>::getRealBackBuffer() {
    if(backgroundLayer)
        return backgroundLayer->getRealBackBuffer();
    return 0;
}

template <typename RGB>
void SmartMatrix3<RGB>::setBackgroundBrightness(uint8_t brightness) {
    if(backgroundLayer)
        backgroundLayer->setBrightness(brightness);
}

template <typename RGB>
void SmartMatrix3<RGB>::enableColorCorrection(bool enabled) {
    if(backgroundLayer)
        backgroundLayer->enableColorCorrection(enabled);
}
