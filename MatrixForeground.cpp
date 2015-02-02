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

#include <string.h>
#include "SmartMatrix.h"

// FIXME: Most of these variable should be in smartmatrix class (i.e. one per instance of smart matrix, rather than being global)
// options
static char text[textLayerMaxStringLength];
static unsigned char textlen;
static int scrollcounter = 0;

static rgb24 textcolor = {0xff, 0xff, 0xff};
static int fontTopOffset = 1;
static int fontLeftOffset = 1;
static bool majorScrollFontChange = false;


bool hasForeground = false;

// Scrolling
static ScrollMode scrollmode = bounceForward;
static unsigned char framesperscroll = 4;

//bitmap size is 32 rows (supporting maximum dimension of screen height in all rotations), by 32 bits
// double buffered to prevent flicker while drawing
static uint32_t foregroundBitmap[2][MATRIX_WIDTH][MATRIX_WIDTH / 32];
const unsigned char foregroundDrawBuffer = 0;
const unsigned char foregroundRefreshBuffer = 1;
volatile bool SmartMatrix::foregroundCopyPending = false;

static const bitmap_font *scrollFont = &apple5x7;

// these variables describe the text bitmap: size, location on the screen, and bounds of where it moves
static unsigned int textWidth;
static int scrollMin, scrollMax;
static int scrollPosition;

// stops the scrolling text on the next refresh
void SmartMatrix::stopScrollText(void) {
    // setup conditions for ending scrolling:
    // scrollcounter is next to zero
    scrollcounter = 1;
    // position text at the end of the cycle
    scrollPosition = scrollMin;
}

void SmartMatrix::clearForeground(void) {
    memset(foregroundBitmap[foregroundDrawBuffer], 0x00, sizeof(foregroundBitmap[0]));
}

void SmartMatrix::displayForegroundDrawing(bool waitUntilComplete) {
    hasForeground = true;

    while (foregroundCopyPending);

    foregroundCopyPending = true;

    while (waitUntilComplete && foregroundCopyPending);
}

void SmartMatrix::handleForegroundDrawingCopy(void) {
    if (!foregroundCopyPending)
        return;

    memcpy(foregroundBitmap[foregroundRefreshBuffer], foregroundBitmap[foregroundDrawBuffer], sizeof(foregroundBitmap[0]));
    redrawForeground();
    foregroundCopyPending = false;
}

void SmartMatrix::drawForegroundPixel(int16_t x, int16_t y, bool opaque) {
    uint32_t tempBitmask;

    if(opaque) {
        tempBitmask = 0x80000000 >> x;
        foregroundBitmap[foregroundDrawBuffer][y][0] |= tempBitmask;
    } else {
        tempBitmask = ~(0x80000000 >> x);
        foregroundBitmap[foregroundDrawBuffer][y][0] &= tempBitmask;
    }
}

bitmap_font *foregroundfont = (bitmap_font *) &apple3x5;


void SmartMatrix::setForegroundFont(fontChoices newFont) {
    foregroundfont = (bitmap_font *)fontLookup(newFont);
    majorScrollFontChange = true;
}

void SmartMatrix::drawForegroundChar(int16_t x, int16_t y, char character, bool opaque) {
    uint32_t tempBitmask;
    int k;

    for (k = y; k < y+foregroundfont->Height; k++) {
        // ignore rows that are not on the screen
        if(k < 0) continue;
        if (k > SmartMatrix::screenConfig.localHeight) return;

        // read in uint8, shift it to be in MSB (font is in the top bits of the uint32)
        tempBitmask = getBitmapFontRowAtXY(character, k - y, foregroundfont) << 24;
        if (x < 0)
            foregroundBitmap[foregroundDrawBuffer][k][0] |= tempBitmask << -x;
        else
            foregroundBitmap[foregroundDrawBuffer][k][0] |= tempBitmask >> x;
    }
}

void SmartMatrix::drawForegroundString(int16_t x, int16_t y, const char text [], bool opaque) {
    // limit text to 10 chars, why?
    for (int i = 0; i < 10; i++) {
        char character = text[i];
        if (character == '\0')
            return;

        drawForegroundChar(i * foregroundfont->Width + x, y, character, opaque);
    }
}

void SmartMatrix::drawForegroundMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *bitmap, bool opaque) {
    int xcnt, ycnt;

    for (ycnt = 0; ycnt < height; ycnt++) {
        for (xcnt = 0; xcnt < width; xcnt++) {
            if (getBitmapPixelAtXY(xcnt, ycnt, width, height, bitmap)) {
                drawForegroundPixel(x + xcnt, y + ycnt, opaque);
            }
        }
    }
}

// returns 0 if stopped
// returns positive number indicating number of loops left if running
// returns -1 if continuously scrolling
int SmartMatrix::getScrollStatus(void) const {
    return scrollcounter;
}

void SmartMatrix::setScrollMinMax(void) {
   switch (scrollmode) {
    case wrapForward:
    case bounceForward:
    case bounceReverse:
    case wrapForwardFromLeft:
        scrollMin = -textWidth;
        scrollMax = screenConfig.localWidth;

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

void SmartMatrix::scrollText(const char inputtext[], int numScrolls) {
    int length = strlen((const char *)inputtext);
    if (length > textLayerMaxStringLength)
        length = textLayerMaxStringLength;
    strncpy(text, (const char *)inputtext, length);
    textlen = length;
    scrollcounter = numScrolls;

    textWidth = (textlen * scrollFont->Width) - 1;

    setScrollMinMax();
 }

//Updates the text that is currently scrolling to the new value
//Useful for a clock display where the time changes.
void SmartMatrix::updateScrollText(const char inputtext[]){
    int length = strlen((const char *)inputtext);
    if (length > textLayerMaxStringLength)
        length = textLayerMaxStringLength;
    strncpy(text, (const char *)inputtext, length);
    textlen = length;
    textWidth = (textlen * scrollFont->Width) - 1;

    setScrollMinMax();
}

// TODO: recompute stuff after changing mode, font, etc
void SmartMatrix::setScrollMode(ScrollMode mode) {
    scrollmode = mode;
}

void SmartMatrix::setScrollSpeed(unsigned char pixels_per_second) {
    framesperscroll = (MATRIX_REFRESH_RATE * 1.0) / pixels_per_second;
}

void SmartMatrix::setScrollFont(fontChoices newFont) {
    scrollFont = fontLookup(newFont);
}

void SmartMatrix::setScrollColor(const rgb24 & newColor) {
    copyRgb24(textcolor, newColor);
}

void SmartMatrix::setScrollOffsetFromTop(int offset) {
    fontTopOffset = offset;
    majorScrollFontChange = true;
}

void SmartMatrix::setScrollStartOffsetFromLeft(int offset) {
    fontLeftOffset = offset;
}


// if font size or position changed since the last call, redraw the whole frame
void SmartMatrix::redrawForeground(void) {
    int j, k;
    int charPosition, textPosition;
    uint8_t charY0, charY1;


    for (j = 0; j < screenConfig.localHeight; j++) {

        // skip rows without text
        if (j < fontTopOffset || j >= fontTopOffset + scrollFont->Height)
            continue;

        hasForeground = true;
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

        if (screenConfig.localHeight < fontTopOffset + scrollFont->Height) {
            charY1 = screenConfig.localHeight - fontTopOffset;
        } else {
            charY1 = scrollFont->Height;
        }

        /* TODO: some edge cases could end up with unwanted drawing to the screen, e.g. foregrounddrawing call,
         * then scrolling text change before displayForegroundDrawing() call would show drawing before intended
         */
        if(majorScrollFontChange) {
            // clear full refresh buffer and copy background over
            memset(foregroundBitmap[foregroundRefreshBuffer], 0x00, sizeof(foregroundBitmap[0]));
            memcpy(foregroundBitmap[foregroundRefreshBuffer], foregroundBitmap[foregroundDrawBuffer], sizeof(foregroundBitmap[0]));
            majorScrollFontChange = false;
        }

        // clear rows used by font before drawing on top
        for (k = 0; k < charY1 - charY0; k++)
            foregroundBitmap[foregroundRefreshBuffer][j + k][0] = 0x00;

        while (textPosition < textlen && charPosition < screenConfig.localWidth) {
            uint32_t tempBitmask;
            // draw character from top to bottom
            for (k = charY0; k < charY1; k++) {
                // read in uint8, shift it to be in MSB (font is in the top bits of the uint32)
                tempBitmask = getBitmapFontRowAtXY(text[textPosition], k, scrollFont) << 24;
                if (charPosition < 0)
                    foregroundBitmap[foregroundRefreshBuffer][j + k - charY0][0] |= tempBitmask << -charPosition;
                else
                    foregroundBitmap[foregroundRefreshBuffer][j + k - charY0][0] |= tempBitmask >> charPosition;
            }

            // get set up for next character
            charPosition += scrollFont->Width;
            textPosition++;
        }

        j += (charY1 - charY0) - 1;
    }
}

// called once per frame to update foreground (virtual) bitmap
// function needs major efficiency improvments
void SmartMatrix::updateForeground(void) {
    bool resetScrolls = false;
    static unsigned char currentframe = 0;

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
        redrawForeground();
    }
}

// returns true and copies color to xyPixel if pixel is opaque, returns false if not
bool SmartMatrix::getForegroundPixel(uint8_t hardwareX, uint8_t hardwareY, rgb24 *xyPixel) {
    uint8_t localScreenX, localScreenY;

    // convert hardware x/y to the pixel in the local screen
    switch( SmartMatrix::screenConfig.rotation ) {
      case rotation0 :
        localScreenX = hardwareX;
        localScreenY = hardwareY;
        break;
      case rotation180 :
        localScreenX = (MATRIX_WIDTH - 1) - hardwareX;
        localScreenY = (MATRIX_HEIGHT - 1) - hardwareY;
        break;
      case  rotation90 :
        localScreenX = hardwareY;
        localScreenY = (MATRIX_WIDTH - 1) - hardwareX;
        break;
      case  rotation270 :
        localScreenX = (MATRIX_HEIGHT - 1) - hardwareY;
        localScreenY = hardwareX;
        break;
      default:
        // TODO: Should throw an error
        return false;
    };

    uint32_t bitmask = 0x01 << (31 - localScreenX);

    if (foregroundBitmap[foregroundRefreshBuffer][localScreenY][0] & bitmask) {
        copyRgb24(*xyPixel, textcolor);
        return true;
    }

    return false;
}
