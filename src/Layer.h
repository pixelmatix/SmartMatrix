#ifndef _LAYER_H_
#define _LAYER_H_

#include "MatrixCommon.h"

class SM_Layer {
    public:
        virtual void frameRefreshCallback();

        // fills refreshRow with matrixWidth values - hardwareY is < matrixHeight, not localHeight
        virtual void fillRefreshRow(uint8_t hardwareY, rgb48 refreshRow[]);
        virtual void fillRefreshRow(uint8_t hardwareY, rgb24 refreshRow[]);

        void setRotation(rotationDegrees newrotation);
        void setRefreshRate(uint8_t newRefreshRate);

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
