#ifndef _LAYER_FOREGROUND_H_
#define _LAYER_FOREGROUND_H_

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

#define SMARTMATRIX_FOREGROUND_OPTIONS_NONE     0

//#define FOREGROUND_DRAWING_ENABLED

// font
#include "MatrixFontCommon.h"

template <typename RGB, unsigned int optionFlags>
class SMLayerForeground : public SM_Layer {
    public:
        SMLayerForeground(uint8_t * bitmap, uint8_t width, uint8_t height);
        void frameRefreshCallback();
        void getRefreshPixel(uint8_t x, uint8_t y, rgb48 &xyPixel);
        void fillRefreshRow(uint8_t hardwareY, rgb48 refreshRow[]);
        void fillRefreshRow(uint8_t hardwareY, rgb24 refreshRow[]);

        void setScrollColor(const RGB & newColor);

        // bitmap size is 32 rows (supporting maximum dimension of screen height in all rotations), by 32 bits
        // double buffered to prevent flicker while drawing
        uint8_t * foregroundBitmap;

        void stopScrollText(void);
        void setForegroundFont(fontChoices newFont);
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

#ifdef FOREGROUND_DRAWING_ENABLED
        void clearForeground(void);
        void displayForegroundDrawing(bool waitUntilComplete);
        void handleForegroundDrawingCopy(void);
        void drawForegroundPixel(int16_t x, int16_t y, bool opaque);
        void drawForegroundChar(int16_t x, int16_t y, char character, bool opaque = true);
        void drawForegroundString(int16_t x, int16_t y, const char text [], bool opaque = true);
        void drawForegroundMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *bitmap, bool opaque = true);
#endif
    private:
        void redrawForeground(void);
        // todo: move somewhere else
        static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);
        void updateForeground(void);

        template <typename RGB_OUT>
        bool getForegroundPixel(uint8_t hardwareX, uint8_t hardwareY, RGB_OUT &xyPixel);

        RGB textcolor = rgb24(0xff, 0xff, 0xff);
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


#ifdef FOREGROUND_DRAWING_ENABLED
        volatile bool foregroundCopyPending = false;


        bitmap_font *foregroundfont = (bitmap_font *) &apple3x5;
#endif
};

#include "Layer_Foreground_Impl.h"

#endif

