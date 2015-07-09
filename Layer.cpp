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

void SM_Layer::addLayer(SM_Layer * newlayer) {
    nextLayer = newlayer;
}
