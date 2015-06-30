/*
 * SmartMatrix Library - Methods for interacting with foreground layer
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

#include "SmartMatrix.h"

void SmartMatrix::stopScrollText(void) {
    foregroundLayerTest.stopScrollText();
}

void SmartMatrix::clearForeground(void) {
    foregroundLayerTest.clearForeground();
}

void SmartMatrix::displayForegroundDrawing(bool waitUntilComplete) {
    foregroundLayerTest.displayForegroundDrawing(waitUntilComplete);
}

void SmartMatrix::drawForegroundPixel(int16_t x, int16_t y, bool opaque) {
    foregroundLayerTest.drawForegroundPixel(x, y, opaque);
}

void SmartMatrix::setForegroundFont(fontChoices newFont) {
    foregroundLayerTest.setForegroundFont(newFont);
}

void SmartMatrix::drawForegroundChar(int16_t x, int16_t y, char character, bool opaque) {
    foregroundLayerTest.drawForegroundChar(x,y,character,opaque);
}

void SmartMatrix::drawForegroundString(int16_t x, int16_t y, const char text [], bool opaque) {
    foregroundLayerTest.drawForegroundString(x,y,text,opaque);
}

void SmartMatrix::drawForegroundMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *bitmap, bool opaque) {
    foregroundLayerTest.drawForegroundMonoBitmap(x,y,width,height,bitmap,opaque);
}

int SmartMatrix::getScrollStatus(void) const {
    return foregroundLayerTest.getScrollStatus();
}

void SmartMatrix::scrollText(const char inputtext[], int numScrolls) {
    foregroundLayerTest.scrollText(inputtext, numScrolls);
}

void SmartMatrix::updateScrollText(const char inputtext[]){
    foregroundLayerTest.updateScrollText(inputtext);
}

void SmartMatrix::setScrollMode(ScrollMode mode) {
    foregroundLayerTest.setScrollMode(mode);
}

void SmartMatrix::setScrollSpeed(unsigned char pixels_per_second) {
    foregroundLayerTest.setScrollSpeed(pixels_per_second);
}

void SmartMatrix::setScrollFont(fontChoices newFont) {
    foregroundLayerTest.setScrollFont(newFont);
}

void SmartMatrix::setScrollColor(const rgb24 & newColor) {
    foregroundLayerTest.setScrollColor(newColor);
}

void SmartMatrix::setScrollOffsetFromTop(int offset) {
    foregroundLayerTest.setScrollOffsetFromTop(offset);
}

void SmartMatrix::setScrollStartOffsetFromLeft(int offset) {
    foregroundLayerTest.setScrollStartOffsetFromLeft(offset);
}
