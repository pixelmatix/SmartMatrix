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
#include "SmartMatrix.h"

const rgb24 SmartMatrix::readPixel(int16_t x, int16_t y) {
    if(backgroundLayerTest)
        return backgroundLayerTest->readPixel(x, y);
    return {0,0,0};
}

void SmartMatrix::drawPixel(int16_t x, int16_t y, const rgb24& color) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawPixel(x,y,color);
}

void SmartMatrix::drawFastHLine(int16_t x0, int16_t x1, int16_t y, const rgb24& color) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawFastHLine(x0, x1, y, color);
}

void SmartMatrix::drawFastVLine(int16_t x, int16_t y0, int16_t y1, const rgb24& color) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawFastVLine(x,y0,y1,color);
}

void SmartMatrix::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const rgb24& color) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawLine(x1,y1,x2,y2,color);
}

void SmartMatrix::drawCircle(int16_t x0, int16_t y0, uint16_t radius, const rgb24& color) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawCircle(x0,y0,radius,color);
}

void SmartMatrix::fillCircle(int16_t x0, int16_t y0, uint16_t radius, const rgb24& outlineColor, const rgb24& fillColor) {
    if(backgroundLayerTest)
        backgroundLayerTest->fillCircle(x0,y0,radius,outlineColor,fillColor);
}

void SmartMatrix::fillCircle(int16_t x0, int16_t y0, uint16_t radius, const rgb24& fillColor) {
    if(backgroundLayerTest)
        backgroundLayerTest->fillCircle(x0, y0, radius,fillColor);
}

void SmartMatrix::drawEllipse(int16_t x0, int16_t y0, uint16_t radiusX, uint16_t radiusY, const rgb24& color) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawEllipse(x0, y0, radiusX, radiusY, color);
}

void SmartMatrix::fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
  uint16_t radius, const rgb24& fillColor) {
    if(backgroundLayerTest)
        backgroundLayerTest->fillRoundRectangle(x0, y0, x1, y1, radius, fillColor);
}

void SmartMatrix::fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
  uint16_t radius, const rgb24& outlineColor, const rgb24& fillColor) {
    if(backgroundLayerTest)
        backgroundLayerTest->fillRoundRectangle(x0, y0, x1, y1, radius, outlineColor, fillColor);
}

void SmartMatrix::drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
  uint16_t radius, const rgb24& outlineColor) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawRoundRectangle(x0, y0, x1, y1, radius, outlineColor);
}

void SmartMatrix::fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const rgb24& fillColor) {
    if(backgroundLayerTest)
        backgroundLayerTest->fillTriangle(x1, y1, x2, y2, x3, y3, fillColor);
}

void SmartMatrix::fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3,
  const rgb24& outlineColor, const rgb24& fillColor) {
    if(backgroundLayerTest)
        backgroundLayerTest->fillTriangle(x1, y1, x2, y2, x3, y3, outlineColor, fillColor);
}

void SmartMatrix::drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const rgb24& color) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawTriangle(x1, y1, x2, y2, x3, y3, color);
}

void SmartMatrix::drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawRectangle(x0, y0, x1, y1, color);
}

void SmartMatrix::fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color) {
    if(backgroundLayerTest)
        backgroundLayerTest->fillRectangle(x0, y0, x1, y1, color);
}

void SmartMatrix::fillScreen(const rgb24& color) {
    if(backgroundLayerTest)
        backgroundLayerTest->fillScreen(color);
}

void SmartMatrix::fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& outlineColor, const rgb24& fillColor) {
    if(backgroundLayerTest)
        backgroundLayerTest->fillRectangle(x0, y0, x1, y1, outlineColor, fillColor);
}

void SmartMatrix::setFont(fontChoices newFont) {
    if(backgroundLayerTest)
        backgroundLayerTest->setFont(newFont);
}

void SmartMatrix::drawChar(int16_t x, int16_t y, const rgb24& charColor, char character) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawChar(x,y,charColor,character);
}

void SmartMatrix::drawString(int16_t x, int16_t y, const rgb24& charColor, const char text[]) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawString(x,y,charColor,text);
}

void SmartMatrix::drawString(int16_t x, int16_t y, const rgb24& charColor, const rgb24& backColor, const char text[]) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawString(x,y,charColor,backColor,text);
}

void SmartMatrix::drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height,
  const rgb24& bitmapColor, const uint8_t *bitmap) {
    if(backgroundLayerTest)
        backgroundLayerTest->drawMonoBitmap(x,y,width,height, bitmapColor, bitmap);
}

void SmartMatrix::swapBuffers(bool copy) {
    if(backgroundLayerTest)
        backgroundLayerTest->swapBuffers(copy);
}

#ifdef SMARTMATRIX_TRIPLEBUFFER
void SmartMatrix::swapBuffersWithInterpolation_frames(int framesToInterpolate, bool copy) {
    if(backgroundLayerTest)
        backgroundLayerTest->swapBuffersWithInterpolation_frames(framesToInterpolate, copy);
}

void SmartMatrix::swapBuffersWithInterpolation_ms(int interpolationSpan_ms, bool copy) {
    if(backgroundLayerTest)
        backgroundLayerTest->swapBuffersWithInterpolation_ms(interpolationSpan_ms, copy);
}
#endif

rgb24 *SmartMatrix::backBuffer(void) {
    if(backgroundLayerTest)
        return backgroundLayerTest->backBuffer();
    return 0;
}

void SmartMatrix::setBackBuffer(rgb24 *newBuffer) {
    if(backgroundLayerTest)
        backgroundLayerTest->setBackBuffer(newBuffer);
}

rgb24 *SmartMatrix::getRealBackBuffer() {
    if(backgroundLayerTest)
        return backgroundLayerTest->getRealBackBuffer();
    return 0;
}

void SmartMatrix::setBackgroundBrightness(uint8_t brightness) {
    if(backgroundLayerTest)
        backgroundLayerTest->setBrightness(brightness);
}

void SmartMatrix::setColorCorrection(colorCorrectionModes mode) {
    if(backgroundLayerTest)
        backgroundLayerTest->setColorCorrection(mode);
}

