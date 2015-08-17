#ifndef _LAYER_H_
#define _LAYER_H_

#include "MatrixCommon.h"

template <typename RGB>
class SM_Layer {
    public:
        virtual void frameRefreshCallback();

        virtual void getRefreshPixel(uint8_t x, uint8_t y, rgb48 &xyPixel);

        void setRotation(rotationDegrees newrotation);
        void setRefreshRate(uint8_t newRefreshRate);
        void addLayer(SM_Layer * newlayer);
        SM_Layer * nextLayer;
    protected:
        rotationDegrees rotation;
        uint8_t matrixWidth, matrixHeight;
        uint8_t localWidth, localHeight;
        uint8_t refreshRate;
        
    private:
};

#include "Layer_Impl.h"

#endif
