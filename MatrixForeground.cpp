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

char tmp[16];

// options
char text[textLayerMaxStringLength];
unsigned char textlen;
int scrollcounter = 0;

rgb24 textcolor = {0xff, 0xff, 0xff};
int fontOffset = 1;

// Scrolling
ScrollMode scrollmode = bounceForward;
unsigned char framesperscroll = 4;

// The foreground is only one color, so the bitmap needs only one bit per location. A 32 pixel-wide
// panel row can therefore be represented with a 32 bit unsigned integer. We store the foreground
// bitmap as row of panels to match the hardware which is laid out in a single row for DMA addressing.
// The drawing area may be a row of panels, a column, or a 2x2 panel square. Drawing routines will
// translate the drawing row, column positions into the hardware row panel positions.
// The foregroundBitmap array is MATRIX_HEIGHT 32 bit values, one array per panel.
uint32_t foregroundBitmap[MATRIX_HEIGHT][MATRIX_WIDTH / 32];

const bitmap_font *scrollFont = &apple5x7;

// these variables describe the text bitmap: size, location on the screen, and bounds of where it moves
unsigned int textWidth;
int scrollMin, scrollMax;
int scrollPosition;

// stops the scrolling text on the next refresh
void SmartMatrix::stopScrollText(void) {
    // setup conditions for ending scrolling:
    // scrollcounter is next to zero
    scrollcounter = 1;
    // position text at the end of the cycle
    scrollPosition = scrollMin;     // TODO: scrollMin may not be initialized?
}

// returns 0 if stopped
// returns positive number indicating number of loops left if running
// returns -1 if continuously scrolling
int SmartMatrix::getScrollStatus(void) {
    return scrollcounter;
}

void SmartMatrix::scrollText(const char inputtext[], int numScrolls) {
    int length = strlen((const char *)inputtext);
    if (length > textLayerMaxStringLength)
        length = textLayerMaxStringLength;
    strncpy(text, (const char *)inputtext, length);
    textlen = length;
    scrollcounter = numScrolls;

    textWidth = (textlen * scrollFont->Width) - 1;

    switch (scrollmode) {
    case wrapForward:
    case bounceForward:
    case bounceReverse:
        scrollMin = -textWidth;
        scrollMax = SmartMatrix::screenConfig.localWidth;

        scrollPosition = scrollMax;

        if (scrollmode == bounceReverse)
            scrollPosition = scrollMin;

        // TODO: handle special case - put content in fixed location if wider than window

        break;

    case stopped:
    case off:
        scrollMin = scrollMax = scrollPosition = 0;
        break;
    }
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

void SmartMatrix::setScrollColor(rgb24 newColor) {
    copyRgb24(&textcolor, &newColor);
}

void SmartMatrix::setScrollOffsetFromEdge(int offset) {
    fontOffset = offset;
}

// Convert the character position and drawing row to the panel hardware column and row.
void SmartMatrix::charPositionToPanelRowColumn(
        int16_t charPosition, int16_t drawingRow, int16_t *panel, int16_t *column, int16_t *row)
{
    int16_t panelRow = drawingRow / PANEL_HEIGHT;  // ie, if 2x2 array of panels, then panelRow = {0, 1}
    int16_t panelCol = charPosition / PANEL_WIDTH;
    *panel = panelRow * PANELS_PER_ROW + panelCol;
    *column = charPosition - panelCol * PANEL_WIDTH;
    *row = drawingRow % PANEL_HEIGHT;
}

void SmartMatrix::redrawForeground(void) {
    int j, k;
    int charPosition, textPosition;
    uint8_t charY0, charY1;

    // clear full bitmap
    memset(foregroundBitmap, 0x00, sizeof(foregroundBitmap));

    for (j = 0; j < SmartMatrix::screenConfig.localHeight; j++) {

        // skip rows without text
        if (j < fontOffset || j >= fontOffset + scrollFont->Height) {
            continue;
        }
        // now in row with text

        // find the position of the first char
        charPosition = scrollPosition;
        textPosition = 0;

        // move to first character at least partially on screen
        while (charPosition + scrollFont->Width < 0 ) {
            charPosition += scrollFont->Width;
            textPosition++;
        }

        // find rows within character bitmap that will be drawn 
        // (0-font->height unless text is partially off screen)
        charY0 = j - fontOffset;

        if (SmartMatrix::screenConfig.localHeight < fontOffset + scrollFont->Height) {
            charY1 = SmartMatrix::screenConfig.localHeight - fontOffset;
        } else {
            charY1 = scrollFont->Height;
        }

        while (textPosition < textlen && charPosition < SmartMatrix::screenConfig.localWidth) {
            uint32_t tempBitmask;
            // draw character from top to bottom
            for (k = charY0; k < charY1; k++) {
                // read in uint8, shift it to be in MSB (font is in the top bits of the uint32)
                tempBitmask = getBitmapFontRowAtXY(text[textPosition], k, scrollFont) << 24;

                int16_t colOnPanel, rowOnPanel, panel;
                int16_t drawingRow = j + k - charY0;
                charPositionToPanelRowColumn(charPosition, drawingRow, &panel, &colOnPanel, &rowOnPanel);
                if (colOnPanel > -8 && colOnPanel < 0) { // partially off left side of panel
                    foregroundBitmap[rowOnPanel][panel] |= tempBitmask << -colOnPanel;
                } else {
                    if (colOnPanel >= 0 && colOnPanel < PANEL_WIDTH - scrollFont->Width) {
                        // character fully on-screen
                        foregroundBitmap[rowOnPanel][panel] |= tempBitmask >> colOnPanel;
                    } else { // partially off right side of panel, so draw on both panels
                        foregroundBitmap[rowOnPanel][panel] |= tempBitmask >> colOnPanel;
                        if (panel < PANELS_PER_ROW - 1) {
                            foregroundBitmap[rowOnPanel][panel+1] |= tempBitmask << PANEL_WIDTH-colOnPanel;
                        }
                    }
                }
            }

            // get set up for next character
            charPosition += scrollFont->Width;
            textPosition++;
        }

        j += (charY1 - charY0) - 1;
    }
}

// called once per frame to update foreground (virtual) bitmap
// function needs major efficiency improvements
void SmartMatrix::updateForeground(void) {
    bool resetScrolls = false;
    static unsigned char currentframe = 0;

    // return if not ready to update
    if (!scrollcounter || ++currentframe <= framesperscroll)
        return;

    currentframe = 0;

    switch (scrollmode) {
    case wrapForward:
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
        scrollPosition = 0;
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
    if (SmartMatrix::screenConfig.rotation == rotation0) {
        localScreenX = hardwareX;
        localScreenY = hardwareY;
    } else if (SmartMatrix::screenConfig.rotation == rotation180) {
        localScreenX = (DRAWING_WIDTH - 1) - hardwareX;
        localScreenY = (DRAWING_HEIGHT - 1) - hardwareY;
    } else if (SmartMatrix::screenConfig.rotation == rotation90) {
        localScreenX = hardwareY;
        localScreenY = (DRAWING_WIDTH - 1) - hardwareX;
    } else { /* if (SmartMatrix::screenConfig.rotation == rotation270)*/
        localScreenX = (DRAWING_HEIGHT - 1) - hardwareY;
        localScreenY = hardwareX;
    }

    // Pull the value from the right bitmap. Because we defined the foreground bit map as
    // a row of panels, finding the right foregroundBitmap location is simpler than the 
    // Drawing translations.
    uint8_t panelIndex = localScreenX / 32;
    uint8_t panelScreenX = localScreenX % 32;
    uint32_t bitmask = 0x01 << (31 - panelScreenX);

    if (foregroundBitmap[localScreenY][panelIndex] & bitmask) {
        copyRgb24(xyPixel, &textcolor);
        return true;
    }

    return false;
}
