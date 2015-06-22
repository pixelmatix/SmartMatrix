#ifndef _LAYER_FOREGROUND_H_
#define _LAYER_FOREGROUND_H_

#include "Arduino.h"

#include "Layer.h"
#include "MatrixCommon.h"

class SMLayerForeground : public SM_Layer {
    public:
        SMLayerForeground();
        void frameRefreshCallback();
        void getRefreshPixel(uint8_t x, uint8_t y, rgb24 &refreshPixel);
        void getRefreshPixel(uint8_t x, uint8_t y, rgb48 &refreshPixel);

};

#endif

