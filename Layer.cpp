#include "Layer.h"

void SM_Layer::updateScreenConfig(screen_config & newConfig) {
    copyScreenConfig(screenConfig, newConfig);
}

void SM_Layer::addLayer(SM_Layer * newlayer) {
    nextLayer = newlayer;
}
