#ifndef _LAYER_BACKGROUND_H_
#define _LAYER_BACKGROUND_H_

#include "Arduino.h"

#include "Layer.h"
#include "MatrixCommon.h"
#include "MatrixFontCommon.h"

class SMLayerBackground : public SM_Layer {
    public:
        SMLayerBackground();
        void frameRefreshCallback();
        void getRefreshPixel(uint8_t x, uint8_t y, rgb24 &refreshPixel);
        void getRefreshPixel(uint8_t x, uint8_t y, rgb48 &refreshPixel);

    private:

};

#endif