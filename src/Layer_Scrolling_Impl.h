/*
 * SmartMatrix Library - Scrolling Layer Class
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

#define SCROLLING_BUFFER_ROW_SIZE   (this->localWidth / 8)
#define SCROLLING_BUFFER_SIZE       (SCROLLING_BUFFER_ROW_SIZE * this->localHeight)

template <typename RGB, unsigned int optionFlags>
SMLayerScrolling<RGB, optionFlags>::SMLayerScrolling(uint8_t * bitmap, uint16_t width, uint16_t height) {
    scrollingBitmap = bitmap;
    this->matrixWidth = width;
    this->matrixHeight = height;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::frameRefreshCallback(void) {
    updateScrollingText();
}

// returns true and copies color to xyPixel if pixel is opaque, returns false if not
template<typename RGB, unsigned int optionFlags> template <typename RGB_OUT>
bool SMLayerScrolling<RGB, optionFlags>::getPixel(uint16_t hardwareX, uint16_t hardwareY, RGB_OUT &xyPixel) {
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

    if (scrollingBitmap[(localScreenY * SCROLLING_BUFFER_ROW_SIZE) + (localScreenX/8)] & bitmask) {
        xyPixel = textcolor;
        return true;
    }

    return false;
}

template<typename RGB, unsigned int optionFlags>
bool SMLayerScrolling<RGB, optionFlags>::getPixel(uint16_t hardwareX, uint16_t hardwareY) {
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

    if (scrollingBitmap[(localScreenY * SCROLLING_BUFFER_ROW_SIZE) + (localScreenX/8)] & bitmask) {
        return true;
    }

    return false;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[]) {
    rgb48 currentPixel;
    int i;

    if(this->ccEnabled)
        colorCorrection(textcolor, currentPixel);
    else
        currentPixel = textcolor;

    for(i=0; i<this->matrixWidth; i++) {
        if(!getPixel(i, hardwareY))
            continue;

        refreshRow[i] = currentPixel;
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[]) {
    rgb24 currentPixel;
    int i;

    if(this->ccEnabled)
        colorCorrection(textcolor, currentPixel);
    else
        currentPixel = textcolor;

    for(i=0; i<this->matrixWidth; i++) {
        if(!getPixel(i, hardwareY))
            continue;

        refreshRow[i] = currentPixel;
    }
}

template<typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::setColor(const RGB & newColor) {
    textcolor = newColor;
}

template<typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::enableColorCorrection(bool enabled) {
    this->ccEnabled = sizeof(RGB) <= 3 ? enabled : false;
}

// stops the scrolling text on the next refresh
template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::stop(void) {
    // setup conditions for ending scrolling:
    // scrollcounter is next to zero
    scrollcounter = 1;
    // position text at the end of the cycle
    scrollPosition = scrollMin;
}

// returns 0 if stopped
// returns positive number indicating number of loops left if running
// returns -1 if continuously scrolling
template <typename RGB, unsigned int optionFlags>
int SMLayerScrolling<RGB, optionFlags>::getStatus(void) const {
    return scrollcounter;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::setMinMax(void) {
   switch (scrollmode) {
    case wrapForward:
    case bounceForward:
    case bounceReverse:
    case wrapForwardFromLeft:
        scrollMin = -textWidth;
        scrollMax = this->localWidth;

        scrollPosition = scrollMax;

        if (scrollmode == bounceReverse)
            scrollPosition = scrollMin;
        else if(scrollmode == wrapForwardFromLeft)
            scrollPosition = fontLeftOffset;

        // TODO: handle special case - put content in fixed location if wider than window

        break;

    case stopped:
    case off:
        scrollMin = scrollMax = scrollPosition = 0;
        break;
    }

}

template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::start(const char inputtext[], int numScrolls) {
    int length = strlen((const char *)inputtext);
    if (length > textLayerMaxStringLength)
        length = textLayerMaxStringLength;
    strncpy(text, (const char *)inputtext, length);
    textlen = length;
    scrollcounter = numScrolls;

    textWidth = (textlen * scrollFont->Width) - 1;

    setMinMax();
 }

//Updates the text that is currently scrolling to the new value
//Useful for a clock display where the time changes.
template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::update(const char inputtext[]){
    int length = strlen((const char *)inputtext);
    if (length > textLayerMaxStringLength)
        length = textLayerMaxStringLength;
    strncpy(text, (const char *)inputtext, length);
    textlen = length;
    textWidth = (textlen * scrollFont->Width) - 1;

    setMinMax();
}

// called once per frame to update (virtual) bitmap
// function needs major efficiency improvments
template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::updateScrollingText(void) {
    bool resetScrolls = false;

    // return if not ready to update
    if (!scrollcounter || ++currentframe <= framesperscroll)
        return;

    currentframe = 0;

    switch (scrollmode) {
    case wrapForward:
    case wrapForwardFromLeft:
        scrollPosition--;
        if (scrollPosition <= scrollMin) {
            scrollPosition = scrollMax;
            if (scrollcounter > 0) scrollcounter--;
        }
        break;

    case bounceForward:
        scrollPosition--;
        if (scrollPosition <= scrollMin) {
            scrollmode = bounceReverse;
            if (scrollcounter > 0) scrollcounter--;
        }
        break;

    case bounceReverse:
        scrollPosition++;
        if (scrollPosition >= scrollMax) {
            scrollmode = bounceForward;
            if (scrollcounter > 0) scrollcounter--;
        }
        break;

    default:
    case stopped:
        scrollPosition = fontLeftOffset;
        resetScrolls = true;
        break;
    }

    // done scrolling - move text off screen and disable
    if (!scrollcounter) {
        resetScrolls = true;
    }

    // for now, fill the bitmap fresh with each update
    // TODO: reset only when necessary, and update just the pixels that need it
    resetScrolls = true;
    if (resetScrolls) {
        redrawScrollingText();
    }
}

// TODO: recompute stuff after changing mode, font, etc
template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::setMode(ScrollMode mode) {
    scrollmode = mode;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    this->refreshRate = newRefreshRate;
    framesperscroll = (this->refreshRate * 1.0) / pixelsPerSecond;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::setSpeed(unsigned char pixels_per_second) {
    pixelsPerSecond = pixels_per_second;
    framesperscroll = (this->refreshRate * 1.0) / pixelsPerSecond;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::setFont(fontChoices newFont) {
    scrollFont = fontLookup(newFont);
}

template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::setOffsetFromTop(int offset) {
    fontTopOffset = offset;
    majorScrollFontChange = true;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::setStartOffsetFromLeft(int offset) {
    fontLeftOffset = offset;
}

// if font size or position changed since the last call, redraw the whole frame
template <typename RGB, unsigned int optionFlags>
void SMLayerScrolling<RGB, optionFlags>::redrawScrollingText(void) {
    int j, k;
    int charPosition, textPosition;
    uint16_t charY0, charY1;


    for (j = 0; j < this->localHeight; j++) {

        // skip rows without text
        if (j < fontTopOffset || j >= fontTopOffset + scrollFont->Height)
            continue;

        // now in row with text
        // find the position of the first char
        charPosition = scrollPosition;
        textPosition = 0;

        // move to first character at least partially on screen
        while (charPosition + scrollFont->Width < 0 ) {
            charPosition += scrollFont->Width;
            textPosition++;
        }

        // find rows within character bitmap that will be drawn (0-font->height unless text is partially off screen)
        charY0 = j - fontTopOffset;

        if (this->localHeight < fontTopOffset + scrollFont->Height) {
            charY1 = this->localHeight - fontTopOffset;
        } else {
            charY1 = scrollFont->Height;
        }

        if(majorScrollFontChange) {
            // clear full refresh buffer before copying background over, size or position may have changed, can't just clear rows used by font
            memset(scrollingBitmap, 0x00, SCROLLING_BUFFER_SIZE);
            majorScrollFontChange = false;
        } else {
            // clear rows used by font before drawing on top
            for (k = 0; k < charY1 - charY0; k++)
                memset(&scrollingBitmap[((j + k) * SCROLLING_BUFFER_ROW_SIZE)], 0x00, SCROLLING_BUFFER_ROW_SIZE);
        }

        while (textPosition < textlen && charPosition < this->localWidth) {
            uint8_t tempBitmask;
            // draw character from top to bottom
            for (k = charY0; k < charY1; k++) {
                tempBitmask = getBitmapFontRowAtXY(text[textPosition], k, scrollFont);
                //tempBitmask = 0xAA;
                if (charPosition < 0) {
                    scrollingBitmap[((j + k - charY0) * SCROLLING_BUFFER_ROW_SIZE) + 0] |= tempBitmask << -charPosition;
                } else {
                    scrollingBitmap[((j + k - charY0) * SCROLLING_BUFFER_ROW_SIZE) + (charPosition/8)] |= tempBitmask >> (charPosition%8);
                    // do two writes if the shifted 8-bit wide bitmask is still on the screen
                    if(charPosition + 8 < this->localWidth && charPosition % 8)
                        scrollingBitmap[((j + k - charY0) * SCROLLING_BUFFER_ROW_SIZE) + (charPosition/8) + 1] |= tempBitmask << (8-(charPosition%8));
                }
            }

            // get set up for next character
            charPosition += scrollFont->Width;
            textPosition++;
        }

        j += (charY1 - charY0) - 1;
    }
}

template <typename RGB, unsigned int optionFlags>
bool SMLayerScrolling<RGB, optionFlags>::getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap) {
    int cell = (y * ((width / 8) + 1)) + (x / 8);

    uint8_t mask = 0x80 >> (x % 8);
    return (mask & bitmap[cell]);
}

