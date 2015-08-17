template <typename RGB>
void SM_Layer<RGB>::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;

    if (rotation == rotation0 || rotation == rotation180) {
        localWidth = matrixWidth;
        localHeight = matrixHeight;
    } else {
        localWidth = matrixHeight;
        localHeight = matrixWidth;
    }
}

template <typename RGB>
void SM_Layer<RGB>::addLayer(SM_Layer * newlayer) {
    nextLayer = newlayer;
}

template <typename RGB>
void SM_Layer<RGB>::setRefreshRate(uint8_t newRefreshRate) {
    refreshRate = newRefreshRate;
}
