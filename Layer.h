#ifndef _LAYER_H_
#define _LAYER_H_

#include "Arduino.h"
#include "MatrixCommon.h"

class SM_Layer {
    public:
        virtual void frameRefreshCallback();
        virtual void getRefreshPixel(uint8_t x, uint8_t y, rgb24 &refreshPixel);
        virtual void getRefreshPixel(uint8_t x, uint8_t y, rgb48 &refreshPixel);
        void updateScreenConfig(screen_config & newConfig);
        screen_config screenConfig;
    protected:
        
    private:


};

#endif
