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
    if(foregroundLayerTest)
        foregroundLayerTest->stopScrollText();
}

void SmartMatrix::clearForeground(void) {
    if(foregroundLayerTest)
        foregroundLayerTest->clearForeground();
}

void SmartMatrix::displayForegroundDrawing(bool waitUntilComplete) {
    if(foregroundLayerTest)
        foregroundLayerTest->displayForegroundDrawing(waitUntilComplete);
}

void SmartMatrix::drawForegroundPixel(int16_t x, int16_t y, bool opaque) {
    if(foregroundLayerTest)
        foregroundLayerTest->drawForegroundPixel(x, y, opaque);
}

void SmartMatrix::setForegroundFont(fontChoices newFont) {
    if(foregroundLayerTest)
        foregroundLayerTest->setForegroundFont(newFont);
}

void SmartMatrix::drawForegroundChar(int16_t x, int16_t y, char character, bool opaque) {
    if(foregroundLayerTest)
        foregroundLayerTest->drawForegroundChar(x,y,character,opaque);
}

void SmartMatrix::drawForegroundString(int16_t x, int16_t y, const char text [], bool opaque) {
    if(foregroundLayerTest)
        foregroundLayerTest->drawForegroundString(x,y,text,opaque);
}

void SmartMatrix::drawForegroundMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *bitmap, bool opaque) {
    if(foregroundLayerTest)
        foregroundLayerTest->drawForegroundMonoBitmap(x,y,width,height,bitmap,opaque);
}

int SmartMatrix::getScrollStatus(void) {
    if(foregroundLayerTest)
        return foregroundLayerTest->getScrollStatus();
    return 0;
}

void SmartMatrix::scrollText(const char inputtext[], int numScrolls) {
    if(foregroundLayerTest)
        foregroundLayerTest->scrollText(inputtext, numScrolls);
}

void SmartMatrix::updateScrollText(const char inputtext[]){
    if(foregroundLayerTest)
        foregroundLayerTest->updateScrollText(inputtext);
}

void SmartMatrix::setScrollMode(ScrollMode mode) {
    if(foregroundLayerTest)
        foregroundLayerTest->setScrollMode(mode);
}

void SmartMatrix::setScrollSpeed(unsigned char pixels_per_second) {
    if(foregroundLayerTest)
        foregroundLayerTest->setScrollSpeed(pixels_per_second);
}

void SmartMatrix::setScrollFont(fontChoices newFont) {
    if(foregroundLayerTest)
        foregroundLayerTest->setScrollFont(newFont);
}

void SmartMatrix::setScrollColor(const rgb24 & newColor) {
    if(foregroundLayerTest)
        foregroundLayerTest->setScrollColor(newColor);
}

void SmartMatrix::setScrollOffsetFromTop(int offset) {
    if(foregroundLayerTest)
        foregroundLayerTest->setScrollOffsetFromTop(offset);
}

void SmartMatrix::setScrollStartOffsetFromLeft(int offset) {
    if(foregroundLayerTest)
        foregroundLayerTest->setScrollStartOffsetFromLeft(offset);
}
