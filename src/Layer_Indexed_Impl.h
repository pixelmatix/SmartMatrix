/*
 * SmartMatrix Library - Indexed Layer Class
 *
 * Copyright (c) 2015 Louis Beaudoin (Pixelmatix)
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

#include <string.h>

const unsigned char indexedDrawBuffer = 0;
const unsigned char indexedRefreshBuffer = 1;

#define INDEXED_BUFFER_ROW_SIZE     (this->localWidth / 8)
#define INDEXED_BUFFER_SIZE         (INDEXED_BUFFER_ROW_SIZE * this->localHeight)

template <typename RGB, unsigned int optionFlags>
SMLayerIndexed<RGB, optionFlags>::SMLayerIndexed(uint8_t * bitmap, uint16_t width, uint16_t height) {
    // size of bitmap is 2 * INDEXED_BUFFER_SIZE
    indexedBitmap = bitmap;
    this->matrixWidth = width;
    this->matrixHeight = height;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::frameRefreshCallback(void) {
    handleBufferCopy();
}

// returns true and copies color to xyPixel if pixel is opaque, returns false if not
template<typename RGB, unsigned int optionFlags> template <typename RGB_OUT>
bool SMLayerIndexed<RGB, optionFlags>::getPixel(uint16_t hardwareX, uint16_t hardwareY, RGB_OUT &xyPixel) {
    uint16_t localScreenX, localScreenY;

    // convert hardware x/y to the pixel in the local screen
    switch( this->rotation ) {
      case rotation0 :
        localScreenX = hardwareX;
        localScreenY = hardwareY;
        break;
      case rotation180 :
        localScreenX = (this->matrixWidth - 1) - hardwareX;
        localScreenY = (this->matrixHeight - 1) - hardwareY;
        break;
      case  rotation90 :
        localScreenX = hardwareY;
        localScreenY = (this->matrixWidth - 1) - hardwareX;
        break;
      case  rotation270 :
        localScreenX = (this->matrixHeight - 1) - hardwareY;
        localScreenY = hardwareX;
        break;
      default:
        // TODO: Should throw an error
        return false;
    };

    uint8_t bitmask = 0x80 >> (localScreenX % 8);

    if (indexedBitmap[(indexedRefreshBuffer * INDEXED_BUFFER_SIZE) + (localScreenY * INDEXED_BUFFER_ROW_SIZE) + (localScreenX/8)] & bitmask) {
        xyPixel = color;
        return true;
    }

    return false;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[]) {
    RGB currentPixel;
    int i;

    if(this->ccEnabled) {
        for(i=0; i<this->matrixWidth; i++) {
            if(!getPixel(i, hardwareY, currentPixel))
                continue;

            colorCorrection(currentPixel, refreshRow[i]);
        }
    } else {
        for(i=0; i<this->matrixWidth; i++) {
            if(!getPixel(i, hardwareY, currentPixel))
                continue;

            // load background pixel without color correction
            refreshRow[i] = currentPixel;
        }
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[]) {
    RGB currentPixel;
    int i;

    if(this->ccEnabled) {
        for(i=0; i<this->matrixWidth; i++) {
            if(!getPixel(i, hardwareY, currentPixel))
                continue;

            colorCorrection(currentPixel, refreshRow[i]);
        }
    } else {
        for(i=0; i<this->matrixWidth; i++) {
            if(!getPixel(i, hardwareY, currentPixel))
                continue;

            // load background pixel without color correction
            refreshRow[i] = currentPixel;
        }
    }
}

template<typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::setIndexedColor(uint8_t index, const RGB & newColor) {
    color = newColor;
}

template<typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::enableColorCorrection(bool enabled) {
    this->ccEnabled = sizeof(RGB) <= 3 ? enabled : false;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::fillScreen(uint8_t index) {
    uint8_t fillValue;
    if(index)
        fillValue = 0xFF;
    else
        fillValue = 0x00;

    memset(&indexedBitmap[indexedDrawBuffer*INDEXED_BUFFER_SIZE], fillValue, INDEXED_BUFFER_SIZE);
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::swapBuffers(bool copy) {
    while (copyPending);

    copyPending = true;

    while (copy && copyPending);
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::handleBufferCopy(void) {
    if (!copyPending)
        return;

    memcpy(&indexedBitmap[indexedRefreshBuffer*INDEXED_BUFFER_SIZE], &indexedBitmap[indexedDrawBuffer*INDEXED_BUFFER_SIZE], INDEXED_BUFFER_SIZE);
    copyPending = false;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawPixel(int16_t x, int16_t y, uint8_t index) {
    uint8_t tempBitmask;

    if(x < 0 || x >= this->localWidth || y < 0 || y >= this->localWidth)
        return;

    if(index) {
        tempBitmask = 0x80 >> (x%8);
        indexedBitmap[indexedDrawBuffer*INDEXED_BUFFER_SIZE + (y * INDEXED_BUFFER_ROW_SIZE) + (x/8)] |= tempBitmask;
    } else {
        tempBitmask = ~(0x80 >> (x%8));
        indexedBitmap[indexedDrawBuffer*INDEXED_BUFFER_SIZE + (y * INDEXED_BUFFER_ROW_SIZE) + (x/8)] &= tempBitmask;
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::setFont(fontChoices newFont) {
    layerFont = (bitmap_font *)fontLookup(newFont);
    majorScrollFontChange = true;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawChar(int16_t x, int16_t y, uint8_t index, char character) {
    uint8_t tempBitmask;
    int k;

    // only draw if character is on the screen
    if (x + scrollFont->Width < 0 || x >= this->localWidth) {
        return;
    }

    for (k = y; k < y+layerFont->Height; k++) {
        // ignore rows that are not on the screen
        if(k < 0) continue;
        if (k >= this->localHeight) return;

        tempBitmask = getBitmapFontRowAtXY(character, k - y, layerFont);
        if (x < 0) {
            indexedBitmap[indexedDrawBuffer*INDEXED_BUFFER_SIZE + (k * INDEXED_BUFFER_ROW_SIZE) + 0] |= tempBitmask << -x;
        } else {
            indexedBitmap[indexedDrawBuffer*INDEXED_BUFFER_SIZE + (k * INDEXED_BUFFER_ROW_SIZE) + (x/8)] |= tempBitmask >> (x%8);
            // do two writes if the shifted 8-bit wide bitmask is still on the screen
            if(x + 8 < this->localWidth && x % 8)
                indexedBitmap[indexedDrawBuffer*INDEXED_BUFFER_SIZE + (k * INDEXED_BUFFER_ROW_SIZE) + (x/8) + 1] |= tempBitmask << (8-(x%8));
        }
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawString(int16_t x, int16_t y, uint8_t index, const char text []) {
    // limit text to 10 chars, why?
    for (int i = 0; i < 10; i++) {
        char character = text[i];
        if (character == '\0')
            return;

        drawChar(i * layerFont->Width + x, y, index, character);
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t index, uint8_t *bitmap) {
    int xcnt, ycnt;

    for (ycnt = 0; ycnt < height; ycnt++) {
        for (xcnt = 0; xcnt < width; xcnt++) {
            if (getBitmapPixelAtXY(xcnt, ycnt, width, height, bitmap)) {
                drawPixel(x + xcnt, y + ycnt, index);
            }
        }
    }
}

template <typename RGB, unsigned int optionFlags>
bool SMLayerIndexed<RGB, optionFlags>::getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap) {
    int cell = (y * ((width / 8) + 1)) + (x / 8);

    uint8_t mask = 0x80 >> (x % 8);
    return (mask & bitmap[cell]);
}

