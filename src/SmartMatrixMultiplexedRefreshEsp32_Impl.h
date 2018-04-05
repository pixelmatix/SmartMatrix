/*
 * SmartMatrix Library - Refresh Code for Teensy 3.x Platform
 *
 * Copyright (c) 2015 Louis Beaudoin (Pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "SmartMatrix3.h"

#if 0
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_heap_caps.h"
#include "i2s_parallel.h"
#endif

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

static void setupTimer(void) {
    // invert OE-PWM output
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    gpio_matrix_out(GPIO_PWM0A_OUT, PWM0_OUT0A_IDX, true, false);

    // setup GPIO for sync from OE-DMA (invert signal)
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_SYNC_0, GPIO_SYNC0_IN);
    gpio_matrix_in(GPIO_SYNC0_IN, PWM0_SYNC0_IN_IDX, true);
    gpio_pulldown_en(GPIO_SYNC0_IN);   //Enable pull down on SYNC0 signal

    // can't use the library function directly as library has hardcoded large prescale values
    //mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);   //Configure PWM0A & PWM0B with above settings

    mcpwm_unit_t mcpwm_num = MCPWM_UNIT_0;
    mcpwm_timer_t timer_num = MCPWM_TIMER_0;

    //esp_err_t mcpwm_init(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num, const mcpwm_config_t *mcpwm_conf)
    {

        #define MATRIX_MCPWM_CLK_PRESCL 0       //MCPWM clock prescale 
        #define MATRIX_TIMER_CLK_PRESCALE 0      //MCPWM timer prescales
        #define MATRIX_MCPWM_BASE_CLK (2 * APB_CLK_FREQ)   //2*APB_CLK_FREQ 160Mhz
        #define MATRIX_MCPWM_CLK (MATRIX_MCPWM_BASE_CLK/(MATRIX_MCPWM_CLK_PRESCL +1))

        mcpwm_dev_t *MCPWM[2] = {&MCPWM0, &MCPWM1};

        periph_module_enable((periph_module_t)(PERIPH_PWM0_MODULE + mcpwm_num));
        MCPWM[mcpwm_num]->clk_cfg.prescale = MATRIX_MCPWM_CLK_PRESCL;

        // set period to max, we never intend for it to wrap, compa to 1 tick
        MCPWM[mcpwm_num]->timer[timer_num].period.prescale = MATRIX_TIMER_CLK_PRESCALE;
        MCPWM[mcpwm_num]->timer[timer_num].period.period = 0xFFFF;
        MCPWM[mcpwm_num]->timer[timer_num].period.upmethod = 0;
        MCPWM[mcpwm_num]->channel[timer_num].cmpr_value[0].cmpr_val = 1;
        MCPWM[mcpwm_num]->channel[timer_num].cmpr_cfg.a_upmethod = 0;

        MCPWM[mcpwm_num]->timer[timer_num].mode.mode = MCPWM_UP_COUNTER;
        mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);

        mcpwm_start(mcpwm_num, timer_num);
        MCPWM[mcpwm_num]->timer_sel.operator0_sel = 0;
        MCPWM[mcpwm_num]->timer_sel.operator1_sel = 1;
        MCPWM[mcpwm_num]->timer_sel.operator2_sel = 2;
        MCPWM[mcpwm_num]->update_cfg.global_up_en = 1;
        MCPWM[mcpwm_num]->update_cfg.global_force_up = 1;
        MCPWM[mcpwm_num]->update_cfg.global_force_up = 0;
    }

    //6. Syncronization configuration
    mcpwm_sync_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_SELECT_SYNC0, 0);    //Load counter value with 20% of period counter of mcpwm timer 1 when sync 0 occurs
}



#define INLINE __attribute__( ( always_inline ) ) inline


// TODO: slower refresh rates require larger timer values - get the min refresh rate from the largest MSB value that will fit in the timer (round up)
#define MIN_REFRESH_RATE    30

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void frameShiftCompleteISR(void);    

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
CircularBuffer SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 60;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
i2s_parallel_buffer_desc_t SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::bufdesc[2][ROWS_PER_FRAME + 1][1<<(COLOR_DEPTH_BITS - LSBMSB_TRANSITION_BIT - 1)];

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::frameStruct * SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateFrames;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrix3RefreshMultiplexed(frameStruct * frameBuffer) {
    matrixUpdateFrames = frameBuffer;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isFrameBufferFree(void) {
    if(cbIsFull(&dmaBuffer))
        return false;
    else
        return true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::frameStruct * SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextFrameBufferPtr(void) {
    return &(matrixUpdateFrames[cbGetNextWrite(&dmaBuffer)]);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeFrameBuffer(uint8_t currentFrame) {
    //SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::frameStruct * currentFramePtr = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextFrameBufferPtr();
    i2s_parallel_flip_to_buffer(&I2S1, cbGetNextWrite(&dmaBuffer));
    cbWrite(&dmaBuffer);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::recoverFromDmaUnderrun(void) {

}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_calc_callback SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrix_calc_callback f) {
    matrixCalcCallback = f;
}

// large factor = more dim, default is full brightness
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dimmingFactor = dimmingMaximum - (100 * 255)/100;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
    dimmingFactor = dimmingMaximum - newBrightness;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    if(newRefreshRate > MIN_REFRESH_RATE)
        refreshRate = newRefreshRate;
    else
        refreshRate = MIN_REFRESH_RATE;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void) {
    cbInit(&dmaBuffer, ESP32_NUM_FRAME_BUFFERS);

    // setup debug output
#ifdef DEBUG_PINS_ENABLED
    gpio_pad_select_gpio(DEBUG_1_GPIO);
    gpio_set_direction(DEBUG_1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DEBUG_1_GPIO, 1);
    gpio_set_level(DEBUG_1_GPIO, 0);
#endif

    setupTimer();

    // completely fill buffer with data before enabling DMA
    matrixCalcCallback();

    // setup DMA linked lists for both frames
    for(int j=0; j<ROWS_PER_FRAME; j++) {
        // first set of data is LSB through MSB, single pass - all color bits are displayed once, which takes care of everything below and inlcluding LSBMSB_TRANSITION_BIT
        bufdesc[0][j][0].memory = &(matrixUpdateFrames[0].rowdata[j].rowbits[0].data);
        bufdesc[0][j][0].size = sizeof(rowBitStruct) * COLOR_DEPTH_BITS;
        bufdesc[1][j][0].memory = &(matrixUpdateFrames[1].rowdata[j].rowbits[0].data);
        bufdesc[1][j][0].size = sizeof(rowBitStruct) * COLOR_DEPTH_BITS;

        int nextBufdescIndex = 1;

        //printf("row %d: \r\n", j);

        for(int i=LSBMSB_TRANSITION_BIT + 1; i<COLOR_DEPTH_BITS; i++) {
            // binary time division setup: we need 2 of bit (LSBMSB_TRANSITION_BIT + 1) four of (LSBMSB_TRANSITION_BIT + 2), etc
            // because we sweep through to MSB each time, it divides the number of times we have to sweep in half (saving linked list RAM)
            // we need 2^(i - LSBMSB_TRANSITION_BIT - 1) == 1 << (i - LSBMSB_TRANSITION_BIT - 1) passes from i to MSB
            //printf("buffer %d: repeat %d times, size: %d, from %d - %d\r\n", nextBufdescIndex, 1<<(i - LSBMSB_TRANSITION_BIT - 1), (COLOR_DEPTH_BITS - i), i, COLOR_DEPTH_BITS-1);

            for(int k=0; k < 1<<(i - LSBMSB_TRANSITION_BIT - 1); k++) {
                bufdesc[0][j][nextBufdescIndex].memory = &(matrixUpdateFrames[0].rowdata[j].rowbits[i].data);
                bufdesc[0][j][nextBufdescIndex].size = sizeof(rowBitStruct) * (COLOR_DEPTH_BITS - i);
                bufdesc[1][j][nextBufdescIndex].memory = &(matrixUpdateFrames[1].rowdata[j].rowbits[i].data);
                bufdesc[1][j][nextBufdescIndex].size = sizeof(rowBitStruct) * (COLOR_DEPTH_BITS - i);
                nextBufdescIndex++;
                //printf("i %d, j %d, k %d\r\n", i, j, k);
            }
        }
    }

    //End markers
    bufdesc[0][ROWS_PER_FRAME][0].memory=NULL;
    bufdesc[1][ROWS_PER_FRAME][0].memory=NULL;
    //printf("\n");

    i2s_parallel_config_t cfg={
        .gpio_bus={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, LAT_PIN, OE_PIN, A_PIN, B_PIN, C_PIN, D_PIN, -1, -1, -1, -1},
        .gpio_clk=CLK_PIN,
        .clkspeed_hz=20*1000*1000,
        .bits=I2S_PARALLEL_BITS_16,
        .bufa=bufdesc[0][0],
        .bufb=bufdesc[1][0],
    };

    //Setup I2S
    i2s_parallel_setup(&I2S1, &cfg);

    setShiftCompleteCallback(frameShiftCompleteISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

    //printf("I2S setup done.\n");
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void frameShiftCompleteISR(void) {
#ifdef DEBUG_PINS_ENABLED
    gpio_set_level(DEBUG_1_GPIO, 1);
#endif
    
    if(!cbIsEmpty(&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer))
        cbRead(&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);
    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback();
    
#ifdef DEBUG_PINS_ENABLED
    gpio_set_level(DEBUG_1_GPIO, 0);
#endif
}
