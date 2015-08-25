#ifndef _LAYER_INDEXED_H_
#define _LAYER_INDEXED_H_

#include "Layer.h"
#include "MatrixCommon.h"

#define SM_INDEXED_OPTIONS_NONE     0

// font
#include "MatrixFontCommon.h"

template <typename RGB, unsigned int optionFlags>
class SMLayerIndexed : public SM_Layer {
    public:
        SMLayerIndexed(uint8_t * bitmap, uint8_t width, uint8_t height);
        void frameRefreshCallback();
        void getRefreshPixel(uint8_t x, uint8_t y, rgb48 &xyPixel);
        void fillRefreshRow(uint8_t hardwareY, rgb48 refreshRow[]);
        void fillRefreshRow(uint8_t hardwareY, rgb24 refreshRow[]);

        // bitmap size is 32 rows (supporting maximum dimension of screen height in all rotations), by 32 bits
        // double buffered to prevent flicker while drawing
        uint8_t * foregroundBitmap;

        void enableColorCorrection(bool enabled);

        void setScrollColor(const RGB & newColor);
        void clearForeground(void);
        void displayForegroundDrawing(bool waitUntilComplete);
        void handleForegroundDrawingCopy(void);
        void drawForegroundPixel(int16_t x, int16_t y, bool opaque);
        void setForegroundFont(fontChoices newFont);
        void drawForegroundChar(int16_t x, int16_t y, char character, bool opaque = true);
        void drawForegroundString(int16_t x, int16_t y, const char text [], bool opaque = true);
        void drawForegroundMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *bitmap, bool opaque = true);

    private:
        // todo: move somewhere else
        static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

        template <typename RGB_OUT>
        bool getForegroundPixel(uint8_t hardwareX, uint8_t hardwareY, RGB_OUT &xyPixel);

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

        volatile bool foregroundCopyPending = false;

        bitmap_font *foregroundfont = (bitmap_font *) &apple3x5;
};

#include "Layer_Indexed_Impl.h"

#endif

