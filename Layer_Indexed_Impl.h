#include <string.h>

const unsigned char foregroundDrawBuffer2 = 0;
const unsigned char foregroundRefreshBuffer2 = 1;

#define FOREGROUND_ROW_SIZE     (this->localWidth / 8)
#define FOREGROUND_BUFFER_SIZE  (FOREGROUND_ROW_SIZE * this->localHeight)

template <typename RGB, unsigned int optionFlags>
SMLayerIndexed<RGB, optionFlags>::SMLayerIndexed(uint8_t * bitmap, uint8_t width, uint8_t height) {
    // size of bitmap is 2 * FOREGROUND_BUFFER_SIZE
    foregroundBitmap = bitmap;
    this->matrixWidth = width;
    this->matrixHeight = height;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::frameRefreshCallback(void) {
    handleForegroundDrawingCopy();
}

// returns true and copies color to xyPixel if pixel is opaque, returns false if not
template<typename RGB, unsigned int optionFlags> template <typename RGB_OUT>
bool SMLayerIndexed<RGB, optionFlags>::getForegroundPixel(uint8_t hardwareX, uint8_t hardwareY, RGB_OUT &xyPixel) {
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

    if (foregroundBitmap[(foregroundRefreshBuffer2 * FOREGROUND_BUFFER_SIZE) + (localScreenY * FOREGROUND_ROW_SIZE) + (localScreenX/8)] & bitmask) {
        xyPixel = textcolor;
        return true;
    }

    return false;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::getRefreshPixel(uint8_t x, uint8_t y, rgb48 &xyPixel) {
    RGB tempPixel;
    if(getForegroundPixel(x, y, tempPixel)) {
        if(this->ccEnabled)
            colorCorrection(tempPixel, xyPixel);
        else
            xyPixel = tempPixel;
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::fillRefreshRow(uint8_t hardwareY, rgb48 refreshRow[]) {
    RGB currentPixel;
    int i;

    if(this->ccEnabled) {
        for(i=0; i<this->matrixWidth; i++) {
            if(!getForegroundPixel(i, hardwareY, currentPixel))
                continue;

            colorCorrection(currentPixel, refreshRow[i]);
        }
    } else {
        for(i=0; i<this->matrixWidth; i++) {
            if(!getForegroundPixel(i, hardwareY, currentPixel))
                continue;

            // load background pixel without color correction
            refreshRow[i] = currentPixel;
        }
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::fillRefreshRow(uint8_t hardwareY, rgb24 refreshRow[]) {
    RGB currentPixel;
    int i;

    if(this->ccEnabled) {
        for(i=0; i<this->matrixWidth; i++) {
            if(!getForegroundPixel(i, hardwareY, currentPixel))
                continue;

            colorCorrection(currentPixel, refreshRow[i]);
        }
    } else {
        for(i=0; i<this->matrixWidth; i++) {
            if(!getForegroundPixel(i, hardwareY, currentPixel))
                continue;

            // load background pixel without color correction
            refreshRow[i] = currentPixel;
        }
    }
}

template<typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::setScrollColor(const RGB & newColor) {
    textcolor = newColor;
}

template<typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::enableColorCorrection(bool enabled) {
    this->ccEnabled = sizeof(RGB) <= 3 ? enabled : false;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::clearForeground(void) {
    memset(&foregroundBitmap[foregroundDrawBuffer2*FOREGROUND_BUFFER_SIZE], 0x00, FOREGROUND_BUFFER_SIZE);
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::displayForegroundDrawing(bool waitUntilComplete) {
    while (foregroundCopyPending);

    foregroundCopyPending = true;

    while (waitUntilComplete && foregroundCopyPending);
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::handleForegroundDrawingCopy(void) {
    if (!foregroundCopyPending)
        return;

    memcpy(&foregroundBitmap[foregroundRefreshBuffer2*FOREGROUND_BUFFER_SIZE], &foregroundBitmap[foregroundDrawBuffer2*FOREGROUND_BUFFER_SIZE], FOREGROUND_BUFFER_SIZE);
    foregroundCopyPending = false;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawForegroundPixel(int16_t x, int16_t y, bool opaque) {
    uint8_t tempBitmask;

    if(x < 0 || x >= this->localWidth || y < 0 || y >= this->localWidth)
        return;

    if(opaque) {
        tempBitmask = 0x80 >> (x%8);
        foregroundBitmap[foregroundDrawBuffer2*FOREGROUND_BUFFER_SIZE + (y * FOREGROUND_ROW_SIZE) + (x/8)] |= tempBitmask;
    } else {
        tempBitmask = ~(0x80 >> (x%8));
        foregroundBitmap[foregroundDrawBuffer2*FOREGROUND_BUFFER_SIZE + (y * FOREGROUND_ROW_SIZE) + (x/8)] &= tempBitmask;
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::setForegroundFont(fontChoices newFont) {
    foregroundfont = (bitmap_font *)fontLookup(newFont);
    majorScrollFontChange = true;
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawForegroundChar(int16_t x, int16_t y, char character, bool opaque) {
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
            foregroundBitmap[foregroundDrawBuffer2*FOREGROUND_BUFFER_SIZE + (k * FOREGROUND_ROW_SIZE) + 0] |= tempBitmask << -x;
        } else {
            foregroundBitmap[foregroundDrawBuffer2*FOREGROUND_BUFFER_SIZE + (k * FOREGROUND_ROW_SIZE) + (x/8)] |= tempBitmask >> (x%8);
            // do two writes if the shifted 8-bit wide bitmask is still on the screen
            if(x + 8 < this->localWidth && x % 8)
                foregroundBitmap[foregroundDrawBuffer2*FOREGROUND_BUFFER_SIZE + (k * FOREGROUND_ROW_SIZE) + (x/8) + 1] |= tempBitmask << (8-(x%8));
        }
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawForegroundString(int16_t x, int16_t y, const char text [], bool opaque) {
    // limit text to 10 chars, why?
    for (int i = 0; i < 10; i++) {
        char character = text[i];
        if (character == '\0')
            return;

        drawForegroundChar(i * foregroundfont->Width + x, y, character, opaque);
    }
}

template <typename RGB, unsigned int optionFlags>
void SMLayerIndexed<RGB, optionFlags>::drawForegroundMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *bitmap, bool opaque) {
    int xcnt, ycnt;

    for (ycnt = 0; ycnt < height; ycnt++) {
        for (xcnt = 0; xcnt < width; xcnt++) {
            if (getBitmapPixelAtXY(xcnt, ycnt, width, height, bitmap)) {
                drawForegroundPixel(x + xcnt, y + ycnt, opaque);
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

