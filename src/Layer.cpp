#include "Layer.h"

void SM_Layer::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;

    if (rotation == rotation0 || rotation == rotation180) {
        localWidth = matrixWidth;
        localHeight = matrixHeight;
    } else {
        localWidth = matrixHeight;
        localHeight = matrixWidth;
    }
}

void SM_Layer::setRefreshRate(uint8_t newRefreshRate) {
    refreshRate = newRefreshRate;
}
