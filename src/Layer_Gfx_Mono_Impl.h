/*
 * SmartMatrix Library - Adafruit_GFX Mono Layer Class
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

#define RGB1_BUFFER_HARDWARE_ROW_SIZE    (this->layerWidth / 8)
#define RGB1_BUFFER_SIZE                 (RGB1_BUFFER_HARDWARE_ROW_SIZE * this->layerHeight)

/* RGB specific methods */
template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::SMLayerGFXMono(uint8_t * bitmap, uint16_t width, uint16_t height, uint16_t layerWidth, uint16_t layerHeight) : Adafruit_GFX(layerWidth, layerHeight) {
    this->matrixWidth = width;
    this->matrixHeight = height;
    this->layerWidth = layerWidth;
    this->layerHeight = layerHeight;
    bufferSize = 2 * RGB1_BUFFER_SIZE;
    indexedBitmap = bitmap;
    this->indexedColor[1] = rgb48(0xffff, 0xffff, 0xffff);
    this->indexedColor[0] = rgb48(0, 0, 0);
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::SMLayerGFXMono(uint16_t width, uint16_t height, uint16_t layerWidth, uint16_t layerHeight) : Adafruit_GFX(layerWidth, layerHeight) {
    this->matrixWidth = width;
    this->matrixHeight = height;
    this->layerWidth = layerWidth;
    this->layerHeight = layerHeight;
    bufferSize = 2 * RGB1_BUFFER_SIZE;
    indexedBitmap = (uint8_t*)malloc(bufferSize);
#ifdef ESP32
    assert(indexedBitmap != NULL);
#else
    this->assert(indexedBitmap != NULL);
#endif
    memset(indexedBitmap, 0x00, 2 * RGB1_BUFFER_SIZE);
    this->indexedColor[1] = rgb48(0xffff, 0xffff, 0xffff);
    this->indexedColor[0] = rgb48(0, 0, 0);
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::begin(void) {
    currentDrawBuffer = 0;
    currentRefreshBuffer = 1;
    swapPending = false;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::handleBufferSwap(void) {
    if (!swapPending)
        return;

    unsigned char newDrawBuffer = currentRefreshBuffer;

    currentRefreshBuffer = currentDrawBuffer;
    currentDrawBuffer = newDrawBuffer;

    swapPending = false;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::frameRefreshCallback(void) {
    updateScrollingText();
    handleBufferSwap();
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags> template <typename RGB_OUT>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::fillRefreshRowTemplated(uint16_t hardwareY, RGB_OUT refreshRow[], int brightnessShifts) {
    RGB_API currentPixel;
    RGB_OUT finalIndexedColor[2];
    int xOffset = 0;
    uint8_t bitmask;
    int i;

    // "i" sweeps across the refresh row with dimensions 0..matrixWidth
    // simplest case: the layer is sized the same as the refresh row and we sweep 0..matrixWidth
    // layer is smaller: we sweep 0..localWidth
    // layer is larger: we sweep 0..matrixWidth
    // layer (same width as refresh row) is offset to the left (layerXOffset < 0): sweep 0..matrixWidth+layerXOffset
    // layer (same width as refresh row) is offset to the right (layerXOffset > 0): sweep layerXOffset..matrixWidth
    // layer is smaller and offset to the left: sweep 0..layerWidth+layerXOffset
    // layer is larger and offset to the left: sweep 0..matrixWidth

    // when the layer starts to the right of the hardware (layerXOffset > 0), "i" sweeps starting from layerXOffset
    int16_t iRangeMin = max((int)0, (int)layerXOffset);

    // we stop sweeping when we run out of layer pixels to read, or hardware pixels to write to
    int16_t iRangeMax = min((int)this->matrixWidth, (int)(this->layerWidth + layerXOffset));

    // our current hardware row is set by hardwareY
    // to map to the row in the current layer:
    // simplest case: the layer is sized the same as the refresh row, no offset, layerY = hardwareY, layerY < layerHeight || layerY < matrixHeight
    // layer is smaller: layerY = hardwareY, we continue if hardwareY < layerHeight
    // layer is larger: layerY = hardwareY, we continue if hardwareY < matrixHeight
    // layer is offset to top (layerYOffset < 0): layerY = hardwareY - layerYOffset, we continue if layerY < layerHeight
    // if the row requested is outside of this layer with the layerYOffset applied, we have nothing to do
    int16_t layerY = hardwareY - layerYOffset;

    if((layerY > (this->layerHeight - 1)) || (layerY < 0))
        return;

    // point to (0, hardwareY)
    uint8_t *ptr = &indexedBitmap[(currentRefreshBuffer * RGB1_BUFFER_SIZE) + (layerY * RGB1_BUFFER_HARDWARE_ROW_SIZE)];

    // xOffset represents the difference between "i" (position in the hardware buffer) and the position in the layer buffer
    xOffset = -layerXOffset;

    // when the layer starts to the left of the hardware, we sweep the layer starting from xOffset
    if(xOffset > 0)
        ptr += xOffset/8;           // we sweep this layer starting from x=xOffset

    // at this point, we need xOffset to be positive and we only care about the remainder, so we add a large multiple of 8 to it
    if(xOffset < 0)
        xOffset += 0x8000;

    // prepare initial position of bitmask (this works because if iRangeMin > 0, xOffset == 0, and vice versa)
    bitmask = 0x80 >> ((iRangeMin + xOffset) % 8);

    if(this->ccEnabled) {
        colorCorrection(indexedColor[0], finalIndexedColor[0]);
        colorCorrection(indexedColor[1], finalIndexedColor[1]);
    } else {
        finalIndexedColor[0] = indexedColor[0];
        finalIndexedColor[1] = indexedColor[1];
    }

    for(i=iRangeMin; i<iRangeMax; i++) {
        bool currentBit = (*ptr & bitmask);

        // every 8 bits, reload mask and ptr++ (this works because if iRangeMin > 0, xOffset == 0, and vice versa)
        if((i + xOffset + 1)%8 == 0) {
            bitmask = 0x80;
            ptr++;
        } else {
            bitmask >>= 1;
        }

        // skip transparent pixels
        if(transparencyEnabled && currentBit == transparentColor)
            continue;

        if(currentBit)
            currentPixel = finalIndexedColor[1];
        else
            currentPixel = finalIndexedColor[0];

        colorCorrection(currentPixel, refreshRow[i]);
    }
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[], int brightnessShifts) {
    fillRefreshRowTemplated(hardwareY, refreshRow, brightnessShifts);
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[], int brightnessShifts) {
    fillRefreshRowTemplated(hardwareY, refreshRow, brightnessShifts);
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::swapBuffers(bool copy) {
    while (swapPending);

    swapPending = true;

    if(copy) {
        while(swapPending);

#if 1
        // workaround for bizarre (optimization) bug - currentDrawBuffer and currentRefreshBuffer are volatile and are changed by an ISR while we're waiting for swapPending here.  They can't be used as parameters to memcpy directly though.  
        if(currentDrawBuffer)
            memcpy(&indexedBitmap[RGB1_BUFFER_SIZE], &indexedBitmap[0], RGB1_BUFFER_SIZE);
        else
            memcpy(&indexedBitmap[0], &indexedBitmap[RGB1_BUFFER_SIZE], RGB1_BUFFER_SIZE);
#else
        // below is untested after copying from backgroundLayer to indexedLayer:

        // Similar code also drawing from volatile variables doesn't work if optimization is turned on: currentDrawBuffer will be equal to currentRefreshBuffer and cause a crash from memcpy copying a buffer to itself.  Why?
        memcpy(&indexedBitmap[currentRefreshBuffer*RGB1_BUFFER_SIZE], &indexedBitmap[currentDrawBuffer*RGB1_BUFFER_SIZE], RGB1_BUFFER_SIZE);

        // this also doesn't work
        //copyRefreshToDrawing();  

        // first checking for (currentDrawBuffer != currentRefreshBuffer) prevents a crash by skipping the copy, but currentDrawBuffer should never be equal to currentRefreshBuffer, except briefly inside an ISR during handleBufferSwap() call
        //if(currentDrawBuffer != currentRefreshBuffer)     
        //   memcpy(backgroundBuffers[currentDrawBuffer], backgroundBuffers[currentRefreshBuffer], sizeof(RGB) * (this->matrixWidth * this->matrixHeight));
#endif
    }
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::setIndexedColor(uint8_t index, const RGB_API & newColor) {
    indexedColor[index % 2] = newColor;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::enableColorCorrection(bool enabled) {
    this->ccEnabled = enabled;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::setRotation(rotationDegrees newrotation) {
    this->layerRotation = newrotation;

    // update Adafruit_GFX rotation
    rotation = (uint8_t)newrotation;

    if(this->layerRotation == rotation0 || this->layerRotation == rotation180) {
        this->localWidth = this->layerWidth;
        this->localHeight = this->layerHeight;
        _width = this->layerWidth;
        _height = this->layerHeight;
    } else {
        this->localWidth = this->layerHeight;
        this->localHeight = this->layerWidth;
        _width = this->layerHeight;
        _height = this->layerWidth;        
    }
}

/* RGB Specific Core Drawing Methods */

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::drawPixel(int16_t x, int16_t y, const rgb1 index) {
    int hwx, hwy;
    uint8_t tempBitmask;

    // check for out of bounds coordinates
    if (x < 0 || y < 0 || x >= this->localWidth || y >= this->localHeight)
        return;

    // map pixel into hardware buffer before writing
    if (this->layerRotation == rotation0) {
        hwx = x;
        hwy = y;
    } else if (this->layerRotation == rotation180) {
        hwx = (this->layerWidth - 1) - x;
        hwy = (this->layerHeight - 1) - y;
    } else if (this->layerRotation == rotation90) {
        hwx = (this->layerWidth - 1) - y;
        hwy = x;
    } else { /* if (layerRotation == rotation270)*/
        hwx = y;
        hwy = (this->layerHeight - 1) - x;
    }

    if(index) {
        tempBitmask = 0x80 >> (hwx%8);
        indexedBitmap[currentDrawBuffer*RGB1_BUFFER_SIZE + (hwy * RGB1_BUFFER_HARDWARE_ROW_SIZE) + (hwx/8)] |= tempBitmask;
    } else {
        tempBitmask = ~(0x80 >> (hwx%8));
        indexedBitmap[currentDrawBuffer*RGB1_BUFFER_SIZE + (hwy * RGB1_BUFFER_HARDWARE_ROW_SIZE) + (hwx/8)] &= tempBitmask;
    }
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::fillScreen(const rgb1 index) {
    uint8_t fillValue;
    if(index)
        fillValue = 0xFF;
    else
        fillValue = 0x00;

    memset(&indexedBitmap[currentDrawBuffer*RGB1_BUFFER_SIZE], fillValue, RGB1_BUFFER_SIZE);
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
int SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::resizeLayer(uint16_t width, uint16_t height) {
    bool resizeWasTooBig = false;

    // resize is going to make what ever's in the refresh buffer look weird, and we're going to need to clear the drawing buffer anyway, so clear them all right now
    clearRefreshAndDrawingBuffers();

    // must be a multiple of 8 for this layer, round up if needed
    width = ROUND_UP_TO_MULTIPLE_OF_8(width);
    height = ROUND_UP_TO_MULTIPLE_OF_8(height);

#if 0
    Serial.print(width);
    Serial.print(",");
    Serial.print(height);
    Serial.print(",");
    Serial.print(bufferSize);
    Serial.println();
#endif
    
    // if dimensions are larger than the available memory, resize to prevent buffer overflow
    if((width * height / 8) > (bufferSize/2)) {
        // if rotation is 90 or 270, then to lower the height of the text we need to lower the width
        if((this->layerRotation % 2)) {
            width = ROUND_DOWN_TO_MULTIPLE_OF_8((bufferSize*8/2)/height);
        } else {
            height = (bufferSize*8/2)/width;
        }
        resizeWasTooBig = true;
    }

    this->layerWidth = width;
    this->layerHeight = height;
    WIDTH = this->layerWidth;
    HEIGHT = this->layerHeight;

    // this will take care of applying changes to Adafruit_GFX _width/_height, and localWidth/Height
    setRotation(this->layerRotation);

    if(resizeWasTooBig)
        return -1;
    else
        return 0;
}

/* RGB Specific Adafruit_GFX methods  */

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::drawPixel(int16_t x, int16_t y, uint16_t color) {
    drawPixel(x, y, (rgb1)(color ? true : false));
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::fillScreen(uint16_t color) {
    rgb1 index = color > 0;
    fillScreen(index);
}

/* RGB Specific SmartMatrix Library 3.0 Backwards Compatibility */

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::drawPixel(int16_t x, int16_t y, uint8_t index) {
    drawPixel(x, y, (rgb1)index);
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::fillScreen(uint8_t index) {
    fillScreen(index > 0);
}

/* RGB Specific (only because indexed color is set separately) SmartMatrix Library 3.0 Backwards Compatibility */

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::drawChar(int16_t x, int16_t y, uint8_t index, char character) {

    // gfxFont = NULL positions xy at top left of font.  Custom gfxFont positions xy at bottom left of font.  SmartMatrix Library used top left, so yAdvance needs to be used for custom font
    int16_t yOffset = 0;

    // yAdvance includes font height plus two spaces between lines, and we subtract one line for the top line of the font
    if(gfxFont)
        yOffset = ((int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance)) - 2 - 1;

    //drawChar(x, y + yOffset, character, ((rgb16)indexedColor[1]).rgb, ((rgb16)indexedColor[1]).rgb, 1);

    setCursor(x, y + yOffset);
    setTextColor(((rgb16)indexedColor[1]).rgb);
    write(character);
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::drawString(int16_t x, int16_t y, uint8_t index, const char text []) {
    // gfxFont = NULL positions xy at top left of font.  Custom gfxFont positions xy at bottom left of font.  SmartMatrix Library used top left, so yAdvance needs to be used for custom font
    int16_t yOffset = 0;

    // yAdvance includes font height plus two spaces between lines, and we subtract one line for the top line of the font
    if(gfxFont)
        yOffset = ((int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance)) - 2 - 1;

    setCursor(x, y + yOffset);
    setTextColor(((rgb16)indexedColor[1]).rgb);
    write(text, strlen(text));
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t index, uint8_t *bitmap) {
    int xcnt, ycnt;

    for (ycnt = 0; ycnt < height; ycnt++) {
        for (xcnt = 0; xcnt < width; xcnt++) {
            if (getBitmapPixelAtXY(xcnt, ycnt, width, height, bitmap)) {
                drawPixel(x + xcnt, y + ycnt, (rgb1)index);
            }
        }
    }
}

/* Common SmartMatrix Library 3.0 Backwards Compatibility */

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::setFont(fontChoices newFont) {
    const GFXfont *gfxFont;

    switch(newFont) {
        default:
        case font3x5:
            gfxFont = &Apple4x6;
            break;
        case font5x7:
            gfxFont = &Apple5x7;
            break;
        case font6x10:
            gfxFont = &Apple6x10;
            break;
        case font8x13:
            gfxFont = &Apple8x13;
            break;
        case gohufont11:
            gfxFont = &GohuFont6x11;
            break;
        case gohufont11b:
            gfxFont = &GohuFont6x11b;
            break;
    }

    setFont(gfxFont);
}

/* Adafruit_GFX methods */

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::setRotation(uint8_t x) {
    setRotation((rotationDegrees)(x & 3));
}

/* Scrolling Text */

#define SHIFT_SCROLL_POSITION_LEFT() (mirrorHardwareScrollingDirection ? scrollPosition++ : scrollPosition--)
#define SHIFT_SCROLL_POSITION_RIGHT() (mirrorHardwareScrollingDirection ? scrollPosition-- : scrollPosition++)
#define IS_SCROLL_POSITION_FULLY_LEFT() (mirrorHardwareScrollingDirection ? (scrollPosition >= scrollMax) : (scrollPosition <= scrollMin))
#define IS_SCROLL_POSITION_FULLY_RIGHT() (mirrorHardwareScrollingDirection ? (scrollPosition <= scrollMin) : (scrollPosition >= scrollMax))
#define SET_SCROLL_POSITION_TO_RIGHT() (mirrorHardwareScrollingDirection ? (scrollPosition = scrollMin) : (scrollPosition = scrollMax) )
#define SET_SCROLL_POSITION_TO_LEFT() (mirrorHardwareScrollingDirection ? (scrollPosition = scrollMax) : (scrollPosition = scrollMin) )
#define SET_SCROLL_POSITION_TO_OFFSET_FROM_LEFT(x) (mirrorHardwareScrollingDirection ? (scrollPosition = scrollMax - textWidth - x) : (scrollPosition = 0 + x) )

// called once per frame to update (virtual) bitmap
// function needs major efficiency improvments
template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::updateScrollingText(void) {
    // return if not ready to update
    if (!scrollcounter || ++currentframe <= framesperscroll)
        return;

    currentframe = 0;

    switch (scrollmode) {
    case wrapForward:
    case wrapForwardFromLeft:
        SHIFT_SCROLL_POSITION_LEFT();
        if (IS_SCROLL_POSITION_FULLY_LEFT()) {
            SET_SCROLL_POSITION_TO_RIGHT();
            if (scrollcounter > 0) scrollcounter--;
        }
        break;

    case bounceForward:
        SHIFT_SCROLL_POSITION_LEFT();
        if (IS_SCROLL_POSITION_FULLY_LEFT()) {
            scrollmode = bounceReverse;
            if (scrollcounter > 0) scrollcounter--;
        }
        break;

    case bounceReverse:
        SHIFT_SCROLL_POSITION_RIGHT();
        if (IS_SCROLL_POSITION_FULLY_RIGHT()) {
            scrollmode = bounceForward;
            if (scrollcounter > 0) scrollcounter--;
        }
        break;

    default:
    case stopped:
        SET_SCROLL_POSITION_TO_OFFSET_FROM_LEFT(fontLeftOffset);
        break;
    }

    // done scrolling - move text off screen and disable
    if (!scrollcounter) {
    }

    if(this->layerRotation == rotation0) {
        layerXOffset = scrollPosition;
        layerYOffset = fontTopOffset;
    } else if(this->layerRotation == rotation180){
        layerXOffset = scrollPosition;
        layerYOffset = (matrixHeight - 1) - (fontTopOffset + layerHeight);
    } else if(this->layerRotation == rotation90){
        layerXOffset = (matrixWidth - 1) - (fontTopOffset + layerWidth);
        layerYOffset = scrollPosition;
    } else { //if(this->layerRotation == rotation270)
        layerXOffset = fontTopOffset;
        layerYOffset = scrollPosition;
    }
}

// returns 0 if stopped
// returns positive number indicating number of loops left if running
// returns -1 if continuously scrolling
template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
int SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::getStatus(void) const {
    return scrollcounter;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
int SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::resizeLayerToText(const char inputtext[]) {
    int16_t x1, y1;
    uint16_t w, h;
    bool resizeWasTooBig = false;

    // size this layer to the text we want to draw
    wrap = false;
    getTextBounds(inputtext, 0, 0, &x1, &y1, &w, &h);
    if(this->layerRotation == rotation90 || this->layerRotation == rotation270)
        resizeWasTooBig = resizeLayer(h, w);
    else
        resizeWasTooBig = resizeLayer(w, h);

    // draw text to the now empty layer
    // set the cursor so the text fits within (0,0..localWidth,localHeight)
    setCursor(-x1, -y1);
    print(inputtext);

    if(resizeWasTooBig)
        return -1;
    else
        return 0;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::getDimensionsOfPrintedString(const char text[], uint16_t *w, uint16_t *h) {
    int16_t x1, y1;
    uint16_t width, height;

    // get the bounds of the text
    wrap = false;
    getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

    // round up to multiple of 8 for width, to match Layer's requirements
    width = ROUND_UP_TO_MULTIPLE_OF_8(width);
    height = ROUND_UP_TO_MULTIPLE_OF_8(height);
    *w = width;
    *h = height;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::clearRefreshAndDrawingBuffers() {
    memset(indexedBitmap, 0x00, RGB1_BUFFER_SIZE*2);
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
int SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::start(const char inputtext[], int numScrolls) {
    bool resizeWasTooBig = resizeLayerToText(inputtext);

    // update layer position and scrolling status
    textWidth = this->localWidth;
    setMinMax();
    scrollcounter = numScrolls;
    currentframe = 0xff-1;  // tell updateScrollingText() we want to update immediately, not in a couple frames
    updateScrollingText();

    // display the text on this layer - don't do a copy as this blocks
#if 0
    // why does this lower the framerate when a bunch of layers are started at once?  isn't swappending=true doing the same thing?
    swapBuffers(false);   
#else
    swapPending = true;
#endif
 
    if(resizeWasTooBig)
        return -1;
    else
        return 0;
}

// Updates the text that is currently scrolling to the new value
// Useful for a clock display where the time changes.
template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
int SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::update(const char inputtext[]){
    bool resizeWasTooBig = resizeLayerToText(inputtext);

    textWidth = this->localWidth;
    setMinMax();
    currentframe = 0xff-1;  // tell updateScrollingText() we want to update immediately, not in a couple frames

    // display the text on this layer - don't do a copy as this blocks
#if 0
    // why does this lower the framerate when a bunch of layers are started at once?  isn't swappending=true doing the same thing?
    swapBuffers(false);   
#else
    swapPending = true;
#endif
 
    if(resizeWasTooBig)
        return -1;
    else
        return 0;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::setMinMax(void) {
    if(this->layerRotation == rotation180 || this->layerRotation == rotation270) {
        mirrorHardwareScrollingDirection = true;
    } else {
        mirrorHardwareScrollingDirection = false;
    }
    switch (scrollmode) {
        case wrapForward:
        case bounceForward:
        case bounceReverse:
        case wrapForwardFromLeft:
            scrollMin = -textWidth;
            scrollMax = (this->layerRotation % 2) ? this->matrixHeight : this->matrixWidth;

            SET_SCROLL_POSITION_TO_RIGHT();

            if (scrollmode == bounceReverse)
                SET_SCROLL_POSITION_TO_LEFT();
            else if(scrollmode == wrapForwardFromLeft)
                SET_SCROLL_POSITION_TO_OFFSET_FROM_LEFT(fontLeftOffset);

            // TODO: handle special case - put content in fixed location if wider than window

            break;

        case stopped:
        case off:
            SET_SCROLL_POSITION_TO_OFFSET_FROM_LEFT(fontLeftOffset);
        break;
    }
}

 // TODO: recompute stuff after changing mode, font, etc
template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::setMode(ScrollMode mode) {
    currentframe = 0xff-1;
    scrollmode = mode;
}

// stops the scrolling text on the next refresh
template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::stop(void) {
    // setup conditions for ending scrolling:
    // scrollcounter is next to zero
    scrollcounter = 1;
    // position text at the end of the cycle
    if(scrollmode == bounceReverse)
        SET_SCROLL_POSITION_TO_RIGHT();
    else
        SET_SCROLL_POSITION_TO_LEFT();
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::setOffsetFromTop(int offset) {
    fontTopOffset = offset;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::setStartOffsetFromLeft(int offset) {
    fontLeftOffset = offset;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::setColor(const RGB_API & newColor) {
    indexedColor[1] = newColor;
}

template <typename RGB_API, typename RGB_STORAGE, unsigned int optionFlags>
void SMLayerGFXMono<RGB_API, RGB_STORAGE, optionFlags>::setSpeed(unsigned char pixels_per_second) {
    pixelsPerSecond = pixels_per_second;
    framesperscroll = (this->refreshRate * 1.0) / pixelsPerSecond;
}
