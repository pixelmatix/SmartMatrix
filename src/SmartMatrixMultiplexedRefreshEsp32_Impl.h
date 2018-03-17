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

//Upper half RGB
#define BIT_R1 (1<<0)   //connected to GPIO2 here
#define BIT_G1 (1<<1)   //connected to GPIO15 here
#define BIT_B1 (1<<2)   //connected to GPIO4 here
//Lower half RGB
#define BIT_R2 (1<<3)   //connected to GPIO16 here
#define BIT_G2 (1<<4)   //connected to GPIO27 here
#define BIT_B2 (1<<5)   //connected to GPIO17 here

#define BIT_A (1<<8)    //connected to GPIO5 here
#define BIT_B (1<<9)    //connected to GPIO18 here
#define BIT_C (1<<10)   //connected to GPIO19 here
#define BIT_D (1<<11)   //connected to GPIO21 here
#define BIT_LAT (1<<12) //connected to GPIO26 here
#define BIT_OE (1<<13)  //connected to GPIO25 here

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#define GPIO_PWM0A_OUT GPIO_NUM_32   //Set GPIO 19 as PWM0A
#define GPIO_SYNC0_IN  GPIO_NUM_34   //Set GPIO 02 as SYNC0

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
void frameCalculationISR(void);

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
CircularBuffer SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 120;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
i2s_parallel_buffer_desc_t SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::bufdesc[ESP32_NUM_FRAME_BUFFERS][1<<(COLOR_DEPTH_BITS)];

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
#if 0
    return &(matrixUpdateFrames[cbGetNextWrite(&dmaBuffer)]);
#else
    return &(matrixUpdateFrames[0]);
#endif
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeFrameBuffer(uint8_t currentFrame) {
    //SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::frameStruct * currentFramePtr = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextFrameBufferPtr();

    // temporary hack? 
    //i2s_parallel_flip_to_buffer(&I2S1, cbGetNextWrite(&dmaBuffer));

    cbWrite(&dmaBuffer);


}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::recoverFromDmaUnderrun(void) {

}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_underrun_callback SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUnderrunCallback;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_calc_callback SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrix_calc_callback f) {
    matrixCalcCallback = f;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixUnderrunCallback(matrix_underrun_callback f) {
    matrixUnderrunCallback = f;
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

    setupTimer();

    // completely fill buffer with data before enabling DMA
    matrixCalcCallback(true);

    // *** TODO: rework this function so a much smaller bufdesc array can be used and we don't run out of memory/address space: lower bits only need one entry

    //Do binary time division setup. Essentially, we need n of plane 0, 2n of plane 1, 4n of plane 2 etc, but that
    //needs to be divided evenly over time to stop flicker from happening. This little bit of code tries to do that
    //more-or-less elegantly.
    int times[COLOR_DEPTH_BITS]={0};
    //printf("Bitplane order: ");
    for (int i=0; i<((1<<COLOR_DEPTH_BITS)-1); i++) {
        int ch=0;
        //Find plane that needs insertion the most
        for (int j=0; j<COLOR_DEPTH_BITS; j++) {
            if (times[j]<=times[ch]) ch=j;
        }
        //printf("%d ", ch);
        //Insert the plane
        for (int j=0; j<2; j++) {
            bufdesc[j][i].memory=&(matrixUpdateFrames[j].framebits[ch]);
            bufdesc[j][i].size=sizeof(frameBitStruct);
        }
        //Magic to make sure we choose this bitplane an appropriate time later next time
        times[ch]+=(1<<(COLOR_DEPTH_BITS-ch));
    }
    //printf("\n");
#if 1

    //End markers
    bufdesc[0][((1<<COLOR_DEPTH_BITS)-1)].memory=NULL;
    bufdesc[1][((1<<COLOR_DEPTH_BITS)-1)].memory=NULL;

    i2s_parallel_config_t cfg={
        .gpio_bus={2, 15, 4, 16, 27, 17, -1, -1, 5, 18, 19, 21, 26, 25, -1, -1},
        .gpio_clk=22,
        .clkspeed_hz=20*1000*1000,
        .bits=I2S_PARALLEL_BITS_16,
        .bufa=bufdesc[0],
        .bufb=bufdesc[1],
    };

    //Setup I2S
    i2s_parallel_setup(&I2S1, &cfg);

    //printf("I2S setup done.\n");

#endif
}

// low priority ISR triggered by software interrupt on a DMA channel that doesn't need interrupts otherwise
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void frameCalculationISR(void) {
    // TODO: figure out how I2S ISR can call this in a low-priority non-blocking interrupt context
    // TODO: figure out if this can be handled as a separate RTOS task, maybe even on a different CPU, without requiring a lot of work from the Arduino application
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void frameShiftCompleteISR(void) {
    // TODO: figure out how I2S can call this as an ISR
}
