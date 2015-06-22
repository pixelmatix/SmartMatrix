#include "Layer.h"

SM_Layer::SM_Layer(void) {

}

void SM_Layer::frameRefreshCallback(void) {

}

void SM_Layer::getRefreshPixel(uint8_t x, uint8_t y, rgb24 &refreshPixel) {

}

void SM_Layer::getRefreshPixel(uint8_t x, uint8_t y, rgb48 &refreshPixel) {

}

void SM_Layer::updateScreenConfig(screen_config & newConfig) {
    copyScreenConfig(screenConfig, newConfig);
}
