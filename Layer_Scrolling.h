#ifndef _LAYER_SCROLLING_H_
#define _LAYER_SCROLLING_H_

#include "Layer.h"
#include "MatrixCommon.h"

// scroll text
const int textLayerMaxStringLength = 100;

typedef enum ScrollMode {
    wrapForward,
    bounceForward,
    bounceReverse,
    stopped,
    off,
    wrapForwardFromLeft,
} ScrollMode;

#define SM_SCROLLING_OPTIONS_NONE     0

// font
#include "MatrixFontCommon.h"

template <typename RGB, unsigned int optionFlags>
class SMLayerScrolling : public SM_Layer {
    public:
        SMLayerScrolling(uint8_t * bitmap, uint8_t width, uint8_t height);
        void frameRefreshCallback();
        void getRefreshPixel(uint8_t x, uint8_t y, rgb48 &xyPixel);
        void fillRefreshRow(uint8_t hardwareY, rgb48 refreshRow[]);
        void fillRefreshRow(uint8_t hardwareY, rgb24 refreshRow[]);

        void setScrollColor(const RGB & newColor);

        // size of bitmap is 1 bit per pixel for width*height (no need for double buffering)
        uint8_t * scrollingBitmap;

        void stopScrollText(void);
        int getScrollStatus(void) const;
        void setScrollMinMax(void);
        void scrollText(const char inputtext[], int numScrolls);
        void updateScrollText(const char inputtext[]);
        void setScrollMode(ScrollMode mode);
        void setScrollSpeed(unsigned char pixels_per_second);
        void setScrollFont(fontChoices newFont);
        void setScrollOffsetFromTop(int offset);
        void setScrollStartOffsetFromLeft(int offset);
        void enableColorCorrection(bool enabled);

    private:
        void redrawScrollingText(void);
        // todo: move somewhere else
        static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);
        void updateScrollingText(void);

        template <typename RGB_OUT>
        bool getPixel(uint8_t hardwareX, uint8_t hardwareY, RGB_OUT &xyPixel);

        RGB textcolor = RGB(0xffff, 0xffff, 0xffff);
        unsigned char currentframe = 0;
        char text[textLayerMaxStringLength];

        unsigned char textlen;
        int scrollcounter = 0;
        const bitmap_font *scrollFont = &apple5x7;

        int fontTopOffset = 1;
        int fontLeftOffset = 1;
        bool majorScrollFontChange = false;

        bool ccEnabled = sizeof(RGB) <= 3 ? true : false;
        ScrollMode scrollmode = bounceForward;
        unsigned char framesperscroll = 4;

        // these variables describe the text bitmap: size, location on the screen, and bounds of where it moves
        unsigned int textWidth;
        int scrollMin, scrollMax;
        int scrollPosition;
};

#include "Layer_Scrolling_Impl.h"

#endif

