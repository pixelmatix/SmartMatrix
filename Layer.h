#ifndef _LAYER_H_
#define _LAYER_H_

#include "Arduino.h"
#include "MatrixCommon.h"

class SM_Layer {
    public:
        SM_Layer();
        void frameRefreshCallback();
        void getRefreshPixel(uint8_t x, uint8_t y, rgb24 &refreshPixel);
        void getRefreshPixel(uint8_t x, uint8_t y, rgb48 &refreshPixel);
        void updateScreenConfig(screen_config & newConfig);
        screen_config screenConfig;
    protected:
        
    private:


};

#endif
