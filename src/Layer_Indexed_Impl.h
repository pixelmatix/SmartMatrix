/*
 * SmartMatrix Library - Indexed Layer Class
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

#include <string.h>

#define INDEXED_BUFFER_ROW_SIZE     (this->localWidth * (indexColourDepth+1) / 8 )
#define INDEXED_BUFFER_SIZE         (INDEXED_BUFFER_ROW_SIZE * this->localHeight)

template <typename RGB, unsigned int optionFlags>
SMLayerIndexed<RGB, optionFlags>::SMLayerIndexed(uint8_t * bitmap, uint16_t width, uint16_t height, SMIndexedLayerColourDepth colourDepthBits) : indexColourDepth(colourDepthBits) {
    // size of bitmap is 2 * INDEXED_BUFFER_SIZE
    //indexColourDepth=colourDepthBits; 
    indexedBitmap = bitmap;
    this->matrixWidth = width;
    this->matrixHeight = height;
    colourLookup=new RGB[(2<<indexColourDepth)-1];
    colourLookup[(2<<indexColourDepth)-1] = rgb48(0xffff, 0xffff, 0xffff);
}

template <typename RGB, unsigned int optionFlags>
SMLayerIndexed<RGB, optionFlags>::SMLayerIndexed(uint16_t width, uint16_t height, SMIndexedLayerColourDepth colourDepthBits) : indexColourDepth(colourDepthBits) {
    // size of bitmap is 2 * INDEXED_BUFFER_SIZE
    indexedBitmap = (uint8_t*)malloc(2 * width * (height / (8/(indexColourDepth+1))));
#ifdef ESP32
    assert(indexedBitmap != NULL);
#else
    //this->assert(indexedBitmap != NULL);
#endif
    memset(indexedBitmap, 0x00, 2 * width * (height / (8/(indexColourDepth+1))));
    this->matrixWidth = width;
    this->matrixHeight = height;
    colourLookup=new RGB[(2<<indexColourDepth)-1];
    colourLookup[(2<<indexColourDepth)-1] = rgb48(0xffff, 0xffff, 0xffff);
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::begin(void) {
    currentDrawBuffer = 0;
    currentRefreshBuffer = 1;
    swapPending = false;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::frameRefreshCallback(void) {
    handleBufferSwap();
}

// returns true and copies color to xyPixel if pixel is opaque, returns false if not
template<typename RGB, unsigned int optionFlags> template <typename RGB_OUT>
bool SMLayerIndexed<RGB, optionFlags>::getPixel(uint16_t hardwareX, uint16_t hardwareY, RGB_OUT &xyPixel) {
    uint32_t localScreenX, localScreenY;

    // convert hardware x/y to the pixel in the local screen
    switch( this->layerRotation ) {
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

    uint32_t bitmask=0;
    uint32_t indxshift;//=(localScreenX % (8/(indexColourDepth+1)));
    uint32_t xbytefind;
    switch(indexColourDepth)
    {
        default:
        case ONEBIT:
            indxshift=localScreenX & 7;
            bitmask=0x80 >> indxshift;
            xbytefind=localScreenX >> 3;
            indxshift=7-indxshift;
            break;
        case TWOBITS:
            indxshift=localScreenX & 3;
            bitmask=0xC0 >> (indxshift << 1);
            xbytefind=localScreenX >> 2;
            indxshift=6-indxshift;
            break;
        case FOURBITS:
            indxshift=localScreenX & 1;
            bitmask=0xF0 >> (indxshift << 2);
            xbytefind=localScreenX >> 1;
            indxshift=4-indxshift;
            break;
        case EIGHTBITS:
            bitmask=0xFF;
            indxshift=0;
            xbytefind=localScreenX;
    }

    uint32_t indx=(indexedBitmap[(currentRefreshBuffer * INDEXED_BUFFER_SIZE) + (localScreenY * INDEXED_BUFFER_ROW_SIZE) + xbytefind] & bitmask);
    if (indx) {
        xyPixel = colourLookup[(indx>>indxshift)-1];
        return true;
    }

    return false;
}



template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[], int brightnessShifts) {
    RGB currentPixel;
    uint32_t i;

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
void SMLayerIndexed<RGB, optionFlags>::fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[], int brightnessShifts) {
    RGB currentPixel;
    uint32_t i;

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
    if(index>(2<<indexColourDepth)-1 || index==0)
        return;

    colourLookup[index-1] = newColor;
}

template<typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::enableColorCorrection(bool enabled) {
    this->ccEnabled = sizeof(RGB) <= 3 ? enabled : false;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::fillScreen(uint8_t index) {

    uint8_t fillValue=0;

    if(index)
    {
        switch(indexColourDepth)
        {
            case ONEBIT:
                fillValue=0xFF;
                break;
            case TWOBITS:
                fillValue=(index & 3) + ((index & 3)<<2) + ((index & 3)<<4) + ((index & 3)<<6);
                break;
            case FOURBITS:
                fillValue=(index & 15) + ((index & 15)<<4);
                break;
            case EIGHTBITS:
                fillValue=index;
                break;
        }
    }

    memset(&indexedBitmap[currentDrawBuffer*INDEXED_BUFFER_SIZE], fillValue, INDEXED_BUFFER_SIZE);
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::swapBuffers(bool copy) {
    while (swapPending);

    swapPending = true;

    if(copy) {
        while(swapPending);

#if 1
        // workaround for bizarre (optimization) bug - currentDrawBuffer and currentRefreshBuffer are volatile and are changed by an ISR while we're waiting for swapPending here.  They can't be used as parameters to memcpy directly though.  
        if(currentDrawBuffer)
            memcpy(&indexedBitmap[INDEXED_BUFFER_SIZE], &indexedBitmap[0], INDEXED_BUFFER_SIZE);
        else
            memcpy(&indexedBitmap[0], &indexedBitmap[INDEXED_BUFFER_SIZE], INDEXED_BUFFER_SIZE);
#else
        // below is untested after copying from backgroundLayer to indexedLayer:

        // Similar code also drawing from volatile variables doesn't work if optimization is turned on: currentDrawBuffer will be equal to currentRefreshBuffer and cause a crash from memcpy copying a buffer to itself.  Why?
        memcpy(&indexedBitmap[currentRefreshBuffer*INDEXED_BUFFER_SIZE], &indexedBitmap[currentDrawBuffer*INDEXED_BUFFER_SIZE], INDEXED_BUFFER_SIZE);

        // this also doesn't work
        //copyRefreshToDrawing();  

        // first checking for (currentDrawBuffer != currentRefreshBuffer) prevents a crash by skipping the copy, but currentDrawBuffer should never be equal to currentRefreshBuffer, except briefly inside an ISR during handleBufferSwap() call
        //if(currentDrawBuffer != currentRefreshBuffer)     
        //   memcpy(backgroundBuffers[currentDrawBuffer], backgroundBuffers[currentRefreshBuffer], sizeof(RGB) * (this->matrixWidth * this->matrixHeight));
#endif
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::handleBufferSwap(void) {
    if (!swapPending)
        return;

    unsigned char newDrawBuffer = currentRefreshBuffer;

    currentRefreshBuffer = currentDrawBuffer;
    currentDrawBuffer = newDrawBuffer;

    swapPending = false;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawPixel(int16_t x, int16_t y, uint8_t index) {

    if(x < 0 || x >= this->localWidth || y < 0 || y >= this->localHeight)
        return;

    uint32_t bitmask;
    uint32_t indxshift;
    uint32_t arrayIndex;

    switch(indexColourDepth)
    {
        case ONEBIT:
            arrayIndex=currentDrawBuffer*INDEXED_BUFFER_SIZE + (y * INDEXED_BUFFER_ROW_SIZE) + (x >> 3);
            indxshift=(x & 7);
            bitmask=0x80 >> indxshift;
            indexedBitmap[arrayIndex]=(indexedBitmap[arrayIndex] & ~bitmask) | ((index & 1) << (7-indxshift));
            break;
        case TWOBITS:
            arrayIndex=currentDrawBuffer*INDEXED_BUFFER_SIZE + (y * INDEXED_BUFFER_ROW_SIZE) + (x >> 2);
            indxshift=(2*(x & 3));
            bitmask=0xC0 >> indxshift;
            indexedBitmap[arrayIndex]=(indexedBitmap[arrayIndex] & ~bitmask) | ((index & 3) << (6-indxshift));
            break;
        case FOURBITS:
            arrayIndex=currentDrawBuffer*INDEXED_BUFFER_SIZE + (y * INDEXED_BUFFER_ROW_SIZE) + (x >> 1);
            indxshift=(4*(x & 1));
            bitmask=0xF0 >> indxshift;
            indexedBitmap[arrayIndex]=(indexedBitmap[arrayIndex] & ~bitmask) | ((index & 15) << (4-indxshift));
            break;
        case EIGHTBITS:
            arrayIndex=currentDrawBuffer*INDEXED_BUFFER_SIZE + (y * INDEXED_BUFFER_ROW_SIZE) + x;
            indexedBitmap[arrayIndex]=index;
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::setFont(fontChoices newFont) {
    layerFont = (bitmap_font *)fontLookup(newFont);
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawChar(int16_t x, int16_t y, uint8_t index, char character, uint8_t backgroundIndex) {
    uint8_t tempBitmask;
    int32_t k;

    // only draw if character is on the screen
    if (x + layerFont->Width < 0 || x >= this->localWidth) {
        return;
    }

    for (k = y; k < y+layerFont->Height; k++) {
        // ignore rows that are not on the screen
        if(k < 0) continue;
        if (k >= this->localHeight) return;

        tempBitmask = getBitmapFontRowAtXY(character, k - y, layerFont);
        drawMonoBitmap(x,k,8,1,index,&tempBitmask,backgroundIndex);


        // if (x < 0) {
        //     indexedBitmap[currentDrawBuffer*INDEXED_BUFFER_SIZE + (k * INDEXED_BUFFER_ROW_SIZE) + 0] |= tempBitmask << -x;
        // } else {
        //     indexedBitmap[currentDrawBuffer*INDEXED_BUFFER_SIZE + (k * INDEXED_BUFFER_ROW_SIZE) + (x/8)] |= tempBitmask >> (x%8);
        //     // do two writes if the shifted 8-bit wide bitmask is still on the screen
        //     if(x + 8 < this->localWidth && x % 8)
        //         indexedBitmap[currentDrawBuffer*INDEXED_BUFFER_SIZE + (k * INDEXED_BUFFER_ROW_SIZE) + (x/8) + 1] |= tempBitmask << (8-(x%8));
        // }
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawString(int16_t x, int16_t y, uint8_t index, const char text [],uint8_t backgroundIndex) {
    uint32_t offset = 0;
    char character;

    while ((character = text[offset++]) != '\0') {
        drawChar(x, y, index, character,backgroundIndex);
        x += layerFont->Width;
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t index, uint8_t *bitmap,uint8_t backgroundIndex) {
    uint32_t xcnt, ycnt;

    for (ycnt = 0; ycnt < height; ycnt++) {
        for (xcnt = 0; xcnt < width; xcnt++) {
            if (getBitmapPixelAtXY(xcnt, ycnt, width, height, bitmap)) {
                drawPixel(x + xcnt, y + ycnt, index);
            } else {
                drawPixel(x + xcnt, y + ycnt, backgroundIndex);
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

