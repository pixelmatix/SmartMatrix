#include <string.h>

const unsigned char foregroundDrawBuffer = 0;
const unsigned char foregroundRefreshBuffer = 1;

#define FOREGROUND_ROW_SIZE     (this->localWidth / 8)
#define FOREGROUND_BUFFER_SIZE  (FOREGROUND_ROW_SIZE * this->localHeight)

template <typename RGB>
SMLayerForeground<RGB>::SMLayerForeground(uint8_t * bitmap, uint8_t width, uint8_t height) {
    // size of bitmap is 2 * FOREGROUND_BUFFER_SIZE
    foregroundBitmap = bitmap;
    this->matrixWidth = width;
    this->matrixHeight = height;
}

template <typename RGB>
void SMLayerForeground<RGB>::frameRefreshCallback(void) {
    handleForegroundDrawingCopy();
    updateForeground();
}

// returns true and copies color to xyPixel if pixel is opaque, returns false if not
template<typename RGB> template <typename RGB_OUT>
bool SMLayerForeground<RGB>::getForegroundPixel(uint8_t hardwareX, uint8_t hardwareY, RGB_OUT &xyPixel) {
    uint8_t localScreenX, localScreenY;

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

    if (foregroundBitmap[(foregroundRefreshBuffer * FOREGROUND_BUFFER_SIZE) + (localScreenY * FOREGROUND_ROW_SIZE) + (localScreenX/8)] & bitmask) {
        xyPixel = textcolor;
        return true;
    }

    return false;
}

template <typename RGB>
void SMLayerForeground<RGB>::getRefreshPixel(uint8_t x, uint8_t y, rgb48 &xyPixel) {
    RGB tempPixel;
    if(getForegroundPixel(x, y, tempPixel)) {
        colorCorrection(this->ccmode, tempPixel, xyPixel);
    }
}


template<typename RGB>
void SMLayerForeground<RGB>::setScrollColor(const RGB & newColor) {
    textcolor = newColor;
}

template <typename RGB>
void SMLayerForeground<RGB>::setColorCorrection(colorCorrectionModes mode) {
    this->ccmode = mode;
}

// stops the scrolling text on the next refresh
template <typename RGB>
void SMLayerForeground<RGB>::stopScrollText(void) {
    // setup conditions for ending scrolling:
    // scrollcounter is next to zero
    scrollcounter = 1;
    // position text at the end of the cycle
    scrollPosition = scrollMin;
}

template <typename RGB>
void SMLayerForeground<RGB>::clearForeground(void) {
    memset(&foregroundBitmap[foregroundDrawBuffer*FOREGROUND_BUFFER_SIZE], 0x00, FOREGROUND_BUFFER_SIZE);
}

template <typename RGB>
void SMLayerForeground<RGB>::displayForegroundDrawing(bool waitUntilComplete) {
    while (foregroundCopyPending);

    foregroundCopyPending = true;

    while (waitUntilComplete && foregroundCopyPending);
}

template <typename RGB>
void SMLayerForeground<RGB>::handleForegroundDrawingCopy(void) {
    if (!foregroundCopyPending)
        return;

    memcpy(&foregroundBitmap[foregroundRefreshBuffer*FOREGROUND_BUFFER_SIZE], &foregroundBitmap[foregroundDrawBuffer*FOREGROUND_BUFFER_SIZE], FOREGROUND_BUFFER_SIZE);
    redrawForeground();
    foregroundCopyPending = false;
}

template <typename RGB>
void SMLayerForeground<RGB>::drawForegroundPixel(int16_t x, int16_t y, bool opaque) {
    uint8_t tempBitmask;

    if(x < 0 || x >= this->localWidth || y < 0 || y >= this->localWidth)
        return;

    if(opaque) {
        tempBitmask = 0x80 >> (x%8);
        foregroundBitmap[foregroundDrawBuffer*FOREGROUND_BUFFER_SIZE + (y * FOREGROUND_ROW_SIZE) + (x/8)] |= tempBitmask;
    } else {
        tempBitmask = ~(0x80 >> (x%8));
        foregroundBitmap[foregroundDrawBuffer*FOREGROUND_BUFFER_SIZE + (y * FOREGROUND_ROW_SIZE) + (x/8)] &= tempBitmask;
    }
}

template <typename RGB>
void SMLayerForeground<RGB>::setForegroundFont(fontChoices newFont) {
    foregroundfont = (bitmap_font *)fontLookup(newFont);
    majorScrollFontChange = true;
}

template <typename RGB>
void SMLayerForeground<RGB>::drawForegroundChar(int16_t x, int16_t y, char character, bool opaque) {
    uint8_t tempBitmask;
    int k;

    // only draw if character is on the screen
    if (x + scrollFont->Width < 0 || x >= this->localWidth) {
        return;
    }

    for (k = y; k < y+foregroundfont->Height; k++) {
        // ignore rows that are not on the screen
        if(k < 0) continue;
        if (k >= this->localHeight) return;

        tempBitmask = getBitmapFontRowAtXY(character, k - y, foregroundfont);
        if (x < 0) {
            foregroundBitmap[foregroundDrawBuffer*FOREGROUND_BUFFER_SIZE + (k * FOREGROUND_ROW_SIZE) + 0] |= tempBitmask << -x;
        } else {
            foregroundBitmap[foregroundDrawBuffer*FOREGROUND_BUFFER_SIZE + (k * FOREGROUND_ROW_SIZE) + (x/8)] |= tempBitmask >> (x%8);
            // do two writes if the shifted 8-bit wide bitmask is still on the screen
            if(x + 8 < this->localWidth && x % 8)
                foregroundBitmap[foregroundDrawBuffer*FOREGROUND_BUFFER_SIZE + (k * FOREGROUND_ROW_SIZE) + (x/8) + 1] |= tempBitmask << (8-(x%8));
        }
    }
}

template <typename RGB>
void SMLayerForeground<RGB>::drawForegroundString(int16_t x, int16_t y, const char text [], bool opaque) {
    // limit text to 10 chars, why?
    for (int i = 0; i < 10; i++) {
        char character = text[i];
        if (character == '\0')
            return;

        drawForegroundChar(i * foregroundfont->Width + x, y, character, opaque);
    }
}

template <typename RGB>
void SMLayerForeground<RGB>::drawForegroundMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *bitmap, bool opaque) {
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
template <typename RGB>
int SMLayerForeground<RGB>::getScrollStatus(void) const {
    return scrollcounter;
}

template <typename RGB>
void SMLayerForeground<RGB>::setScrollMinMax(void) {
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

template <typename RGB>
void SMLayerForeground<RGB>::scrollText(const char inputtext[], int numScrolls) {
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
template <typename RGB>
void SMLayerForeground<RGB>::updateScrollText(const char inputtext[]){
    int length = strlen((const char *)inputtext);
    if (length > textLayerMaxStringLength)
        length = textLayerMaxStringLength;
    strncpy(text, (const char *)inputtext, length);
    textlen = length;
    textWidth = (textlen * scrollFont->Width) - 1;

    setScrollMinMax();
}

// called once per frame to update foreground (virtual) bitmap
// function needs major efficiency improvments
template <typename RGB>
void SMLayerForeground<RGB>::updateForeground(void) {
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
        redrawForeground();
    }
}

// TODO: recompute stuff after changing mode, font, etc
template <typename RGB>
void SMLayerForeground<RGB>::setScrollMode(ScrollMode mode) {
    scrollmode = mode;
}

// TODO:need to get refresh rate from main class


template <typename RGB>
void SMLayerForeground<RGB>::setScrollSpeed(unsigned char pixels_per_second) {
    framesperscroll = (this->refreshRate * 1.0) / pixels_per_second;
}

template <typename RGB>
void SMLayerForeground<RGB>::setScrollFont(fontChoices newFont) {
    scrollFont = fontLookup(newFont);
}

template <typename RGB>
void SMLayerForeground<RGB>::setScrollOffsetFromTop(int offset) {
    fontTopOffset = offset;
    majorScrollFontChange = true;
}

template <typename RGB>
void SMLayerForeground<RGB>::setScrollStartOffsetFromLeft(int offset) {
    fontLeftOffset = offset;
}

// if font size or position changed since the last call, redraw the whole frame
template <typename RGB>
void SMLayerForeground<RGB>::redrawForeground(void) {
    int j, k;
    int charPosition, textPosition;
    uint8_t charY0, charY1;


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

        /* TODO: some edge cases could end up with unwanted drawing to the screen, e.g. foregrounddrawing call,
         * then scrolling text change before displayForegroundDrawing() call would show drawing before intended
         */
        if(majorScrollFontChange) {
            // clear full refresh buffer and copy background over
            // why clear before copy?
            memset(&foregroundBitmap[foregroundRefreshBuffer*FOREGROUND_BUFFER_SIZE], 0x00, FOREGROUND_BUFFER_SIZE);
            //memcpy(&foregroundBitmap[foregroundRefreshBuffer*FOREGROUND_BUFFER_SIZE], &foregroundBitmap[foregroundDrawBuffer*FOREGROUND_BUFFER_SIZE], FOREGROUND_BUFFER_SIZE);
            majorScrollFontChange = false;
        }

        // clear rows used by font before drawing on top
        for (k = 0; k < charY1 - charY0; k++)
            memset(&foregroundBitmap[foregroundRefreshBuffer*FOREGROUND_BUFFER_SIZE + ((j + k) * FOREGROUND_ROW_SIZE)], 0x00, FOREGROUND_ROW_SIZE);

        while (textPosition < textlen && charPosition < this->localWidth) {
            uint8_t tempBitmask;
            // draw character from top to bottom
            for (k = charY0; k < charY1; k++) {
                tempBitmask = getBitmapFontRowAtXY(text[textPosition], k, scrollFont);
                //tempBitmask = 0xAA;
                if (charPosition < 0) {
                    foregroundBitmap[foregroundRefreshBuffer*FOREGROUND_BUFFER_SIZE + ((j + k - charY0) * FOREGROUND_ROW_SIZE) + 0] |= tempBitmask << -charPosition;
                } else {
                    foregroundBitmap[foregroundRefreshBuffer*FOREGROUND_BUFFER_SIZE + ((j + k - charY0) * FOREGROUND_ROW_SIZE) + (charPosition/8)] |= tempBitmask >> (charPosition%8);
                    // do two writes if the shifted 8-bit wide bitmask is still on the screen
                    if(charPosition + 8 < this->localWidth && charPosition % 8)
                        foregroundBitmap[foregroundRefreshBuffer*FOREGROUND_BUFFER_SIZE + ((j + k - charY0) * FOREGROUND_ROW_SIZE) + (charPosition/8) + 1] |= tempBitmask << (8-(charPosition%8));
                }
            }

            // get set up for next character
            charPosition += scrollFont->Width;
            textPosition++;
        }

        j += (charY1 - charY0) - 1;
    }
}

template <typename RGB>
bool SMLayerForeground<RGB>::getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap) {
    int cell = (y * ((width / 8) + 1)) + (x / 8);

    uint8_t mask = 0x80 >> (x % 8);
    return (mask & bitmap[cell]);
}

