#include "Layer.h"


void SM_Layer::updateScreenConfig(screen_config & newConfig) {
    copyScreenConfig(screenConfig, newConfig);
}
