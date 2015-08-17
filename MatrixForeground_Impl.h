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

template <typename RGB>
void SmartMatrix<RGB>::stopScrollText(void) {
    if(foregroundLayer)
        foregroundLayer->stopScrollText();
}

template <typename RGB>
void SmartMatrix<RGB>::clearForeground(void) {
    if(foregroundLayer)
        foregroundLayer->clearForeground();
}

template <typename RGB>
void SmartMatrix<RGB>::displayForegroundDrawing(bool waitUntilComplete) {
    if(foregroundLayer)
        foregroundLayer->displayForegroundDrawing(waitUntilComplete);
}

template <typename RGB>
void SmartMatrix<RGB>::drawForegroundPixel(int16_t x, int16_t y, bool opaque) {
    if(foregroundLayer)
        foregroundLayer->drawForegroundPixel(x, y, opaque);
}

template <typename RGB>
void SmartMatrix<RGB>::setForegroundFont(fontChoices newFont) {
    if(foregroundLayer)
        foregroundLayer->setForegroundFont(newFont);
}

template <typename RGB>
void SmartMatrix<RGB>::drawForegroundChar(int16_t x, int16_t y, char character, bool opaque) {
    if(foregroundLayer)
        foregroundLayer->drawForegroundChar(x,y,character,opaque);
}

template <typename RGB>
void SmartMatrix<RGB>::drawForegroundString(int16_t x, int16_t y, const char text [], bool opaque) {
    if(foregroundLayer)
        foregroundLayer->drawForegroundString(x,y,text,opaque);
}

template <typename RGB>
void SmartMatrix<RGB>::drawForegroundMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *bitmap, bool opaque) {
    if(foregroundLayer)
        foregroundLayer->drawForegroundMonoBitmap(x,y,width,height,bitmap,opaque);
}

template <typename RGB>
int SmartMatrix<RGB>::getScrollStatus(void) {
    if(foregroundLayer)
        return foregroundLayer->getScrollStatus();
    return 0;
}

template <typename RGB>
void SmartMatrix<RGB>::scrollText(const char inputtext[], int numScrolls) {
    if(foregroundLayer)
        foregroundLayer->scrollText(inputtext, numScrolls);
}

template <typename RGB>
void SmartMatrix<RGB>::updateScrollText(const char inputtext[]){
    if(foregroundLayer)
        foregroundLayer->updateScrollText(inputtext);
}

template <typename RGB>
void SmartMatrix<RGB>::setScrollMode(ScrollMode mode) {
    if(foregroundLayer)
        foregroundLayer->setScrollMode(mode);
}

template <typename RGB>
void SmartMatrix<RGB>::setScrollSpeed(unsigned char pixels_per_second) {
    if(foregroundLayer)
        foregroundLayer->setScrollSpeed(pixels_per_second);
}

template <typename RGB>
void SmartMatrix<RGB>::setScrollFont(fontChoices newFont) {
    if(foregroundLayer)
        foregroundLayer->setScrollFont(newFont);
}

template <typename RGB>
void SmartMatrix<RGB>::setScrollColor(const RGB & newColor) {
    if(foregroundLayer)
        foregroundLayer->setScrollColor(newColor);
}

template <typename RGB>
void SmartMatrix<RGB>::setScrollOffsetFromTop(int offset) {
    if(foregroundLayer)
        foregroundLayer->setScrollOffsetFromTop(offset);
}

template <typename RGB>
void SmartMatrix<RGB>::setScrollStartOffsetFromLeft(int offset) {
    if(foregroundLayer)
        foregroundLayer->setScrollStartOffsetFromLeft(offset);
}
