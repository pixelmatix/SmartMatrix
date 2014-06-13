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

#define MAX_SCROLL_LAYERS 5

// options
char text[MAX_SCROLL_LAYERS][textLayerMaxStringLength];
unsigned char textlen[MAX_SCROLL_LAYERS];
int scrollcounter[MAX_SCROLL_LAYERS];

int currentFrame[MAX_SCROLL_LAYERS];
bool updateLayer[MAX_SCROLL_LAYERS];

rgb24 textcolor[MAX_SCROLL_LAYERS];
int fontOffset[MAX_SCROLL_LAYERS];

int indexUsed=-1;

// Scrolling
ScrollMode scrollmode[MAX_SCROLL_LAYERS] ;
unsigned char framesperscroll[MAX_SCROLL_LAYERS];

//bitmap size is 32 rows (supporting maximum dimension of screen height in all rotations), by 32 bits
uint32_t foregroundBitmap[MAX_SCROLL_LAYERS][32][32 / 32];

const bitmap_font *scrollFont[MAX_SCROLL_LAYERS] ; //= {&apple5x7,&apple5x7};

// these variables describe the text bitmap: size, location on the screen, and bounds of where it moves
unsigned int textWidth[MAX_SCROLL_LAYERS];
int scrollMin[MAX_SCROLL_LAYERS], scrollMax[MAX_SCROLL_LAYERS];
int scrollPosition[MAX_SCROLL_LAYERS];

// stops the scrolling text on the next refresh
void SmartMatrix::stopScrollText(int index) {
  // setup conditions for ending scrolling:
  // scrollcounter is next to zero
  scrollcounter[index] = 1;
  // position text at the end of the cycle
  scrollPosition[index] = scrollMin[index];
}

// returns 0 if stopped
// returns positive number indicating number of loops left if running
// returns -1 if continuously scrolling
int SmartMatrix::getScrollStatus(int index) {
  return scrollcounter[index];
}

int SmartMatrix::initNewScrollLayer(void){
  int layerID = indexUsed + 1;
  strncpy(text[layerID],"",0) ;
  textlen[layerID] = strlen(text[layerID]);
  scrollcounter[layerID] = 0;
  textcolor[layerID] = {0xff,0xff,0xff};
  fontOffset[layerID] = 1;
  scrollmode[layerID] = bounceForward;
  framesperscroll[layerID] = 4;
  scrollFont[layerID] = &apple5x7;
  textWidth[layerID] = (textlen[layerID] * scrollFont[layerID]->Width) - 1;
  scrollMin[layerID] = -textWidth[layerID];
  scrollMax[layerID] = SmartMatrix::screenConfig.localWidth;
  scrollPosition[layerID] = scrollMax[layerID];
  updateLayer[layerID] = true;
  currentFrame[layerID] = 0;

  indexUsed = layerID;

  return indexUsed;
}

void SmartMatrix::scrollText(const char inputtext[], int numScrolls, int index) {

  int length = strlen((const char *)inputtext);
  if (length > textLayerMaxStringLength)
  length = textLayerMaxStringLength;
  strncpy(text[index], (const char *)inputtext, length);
  textlen[index] = length;
  scrollcounter[index] = numScrolls;

  textWidth[index] = (textlen[index] * scrollFont[index]->Width) - 1;

  switch (scrollmode[index]) {
    case wrapForward:
    case bounceForward:
    case bounceReverse:
    scrollMin[index] = -textWidth[index];
    scrollMax[index] = SmartMatrix::screenConfig.localWidth;

    scrollPosition[index] = scrollMax[index];

    if (scrollmode[index] == bounceReverse)
    scrollPosition[index] = scrollMin[index];

    // TODO: handle special case - put content in fixed location if wider than window

    break;

    case stopped:
    case off:
    scrollMin[index] = scrollMax[index] = scrollPosition[index] = 0;
    break;
  }
  if(index>indexUsed)indexUsed=index;
}

//Updates the text that is currently scrolling to the new value
//Useful for a clock display where the time changes.
void SmartMatrix::updateScrollText(const char inputtext[], int index){
  int length = strlen((const char *)inputtext);
  if (length > textLayerMaxStringLength)
  length = textLayerMaxStringLength;
  strncpy(text[index], (const char *)inputtext, length);
  textlen[index] = length;
  textWidth[index] = (textlen[index] * scrollFont[index]->Width) - 1;
}

// TODO: recompute stuff after changing mode, font, etc
void SmartMatrix::setScrollMode(ScrollMode mode, int index) {
  scrollmode[index] = mode;
}

void SmartMatrix::setScrollSpeed(unsigned char pixels_per_second, int index) {
  framesperscroll[index] = (MATRIX_REFRESH_RATE * 1.0) / pixels_per_second;
}

void SmartMatrix::setScrollFont(fontChoices newFont, int index) {
  scrollFont[index] = fontLookup(newFont);
}

void SmartMatrix::setScrollColor(rgb24 newColor, int index) {
  copyRgb24(&textcolor[index], &newColor);
}

void SmartMatrix::setScrollOffsetFromEdge(int offset, int index) {
  fontOffset[index] = offset;
}

void SmartMatrix::redrawForeground(void) {

  int j, k;
  int charPosition, textPosition;
  uint8_t charY0, charY1;

  for (int q = 0; q <= indexUsed; q++){


    // clear full bitmap
    memset(foregroundBitmap[q], 0x00, sizeof(foregroundBitmap[q]));

    for (j = 0; j < SmartMatrix::screenConfig.localHeight; j++) {

      // skip rows without text
      if (j < fontOffset[q] || j >= fontOffset[q] + scrollFont[q]->Height)
      continue;

      // now in row with text
      // find the position of the first char
      charPosition = scrollPosition[q];
      textPosition = 0;

      // move to first character at least partially on screen
      while (charPosition + scrollFont[q]->Width < 0 ) {
        charPosition += scrollFont[q]->Width;
        textPosition++;
      }

      // find rows within character bitmap that will be drawn (0-font->height unless text is partially off screen)
      charY0 = j - fontOffset[q];

      if (SmartMatrix::screenConfig.localHeight < fontOffset[q] + scrollFont[q]->Height) {
        charY1 = SmartMatrix::screenConfig.localHeight - fontOffset[q];
      } else {
        charY1 = scrollFont[q]->Height;
      }

      while (textPosition < textlen[q] && charPosition < SmartMatrix::screenConfig.localWidth) {
        uint32_t tempBitmask;
        // draw character from top to bottom
        for (k = charY0; k < charY1; k++) {
          // read in uint8, shift it to be in MSB (font is in the top bits of the uint32)
          tempBitmask = getBitmapFontRowAtXY(text[q][textPosition], k, scrollFont[q]) << 24;

          if (charPosition < 0)
            foregroundBitmap[q][j + k - charY0][0] |= tempBitmask << -charPosition;
          else
            foregroundBitmap[q][j + k - charY0][0] |= tempBitmask >> charPosition;

        }

        // get set up for next character
        charPosition += scrollFont[q]->Width;
        textPosition++;
      }

      j += (charY1 - charY0) - 1;
    }
    //}
  }
}

// called once per frame to update foreground (virtual) bitmap
// function needs major efficiency improvments
void SmartMatrix::updateForeground(void) {
  bool resetScrolls = false;
  //static unsigned char currentframe = 0;

  int returnForAll=0;

  for(int q=0 ; q<=indexUsed; q++){

    // return if not ready to update
    if (!scrollcounter[q] || ++currentFrame[q] <= framesperscroll[q]){
      returnForAll++;
      if(returnForAll==indexUsed+1)return;
      continue;
    }


    currentFrame[q] = 0;

    switch (scrollmode[q]) {
      case wrapForward:
      scrollPosition[q]--;
      if (scrollPosition[q] <= scrollMin[q]) {
        scrollPosition[q] = scrollMax[q];
        if (scrollcounter[q] > 0) scrollcounter[q]--;
      }
      break;

      case bounceForward:
      scrollPosition[q]--;
      if (scrollPosition[q] <= scrollMin[q]) {
        scrollmode[q] = bounceReverse;
        if (scrollcounter[q] > 0) scrollcounter[q]--;
      }
      break;

      case bounceReverse:
      scrollPosition[q]++;
      if (scrollPosition[q] >= scrollMax[q]) {
        scrollmode[q] = bounceForward;
        if (scrollcounter[q] > 0) scrollcounter[q]--;
      }
      break;

      default:
      case stopped:
      scrollPosition[q] = 0;
      resetScrolls = true;
      break;
    }

    // done scrolling - move text off screen and disable
    if (!scrollcounter[q]) {
      resetScrolls = true;
    }

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
    localScreenX = (MATRIX_WIDTH - 1) - hardwareX;
    localScreenY = (MATRIX_HEIGHT - 1) - hardwareY;
  } else if (SmartMatrix::screenConfig.rotation == rotation90) {
    localScreenX = hardwareY;
    localScreenY = (MATRIX_WIDTH - 1) - hardwareX;
  } else { /* if (SmartMatrix::screenConfig.rotation == rotation270)*/
    localScreenX = (MATRIX_HEIGHT - 1) - hardwareY;
    localScreenY = hardwareX;
  }

  uint32_t bitmask = 0x01 << (31 - localScreenX);

  bool any=false;

  for(int q = 0; q<=indexUsed; q++){
    if (foregroundBitmap[q][localScreenY][0] & bitmask) {

      copyRgb24(xyPixel, &textcolor[q]);
      any=true;

    }
    //return true;
  }

  return any;
}
