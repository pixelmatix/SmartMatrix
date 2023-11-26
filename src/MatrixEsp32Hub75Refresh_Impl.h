/*
 * SmartMatrix Library - ESP32 HUB75 Panel Refresh Class
 *
 * Copyright (c) 2020 Louis Beaudoin (Pixelmatix)
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

#include "SmartMatrix.h"

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

#include "Esp32MemDisplay.h"
#include "rom/lldesc.h"

#define INLINE __attribute__( ( always_inline ) ) inline


// TODO: slower refresh rates require larger timer values - get the min refresh rate from the largest MSB value that will fit in the timer (round up)
#define MIN_REFRESH_RATE    30

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void frameShiftCompleteISR(void);    

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
CircularBuffer_SM SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 120;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::minRefreshRate = 120;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint8_t SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::lsbMsbTransitionBit = 0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::frameStruct * SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateFrames[ESP32_NUM_FRAME_BUFFERS];

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrixHub75Refresh(void) {
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isFrameBufferFree(void) {
    if(cbIsFull(&dmaBuffer))
        return false;
    else
        return true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::frameStruct * SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextFrameBufferPtr(void) {
    return matrixUpdateFrames[cbGetNextWrite(&dmaBuffer)];
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeFrameBuffer(uint8_t currentFrame) {
    //SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::frameStruct * currentFramePtr = SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextFrameBufferPtr();
    i2s_parallel_flip_to_buffer(&I2S1, cbGetNextWrite(&dmaBuffer));
    cbWrite(&dmaBuffer);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::recoverFromDmaUnderrun(void) {

}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_calc_callback SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrix_calc_callback f) {
    setShiftCompleteCallback(f);
    matrixCalcCallback = f;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint16_t newRefreshRate) {
    if(newRefreshRate > MIN_REFRESH_RATE)
        minRefreshRate = newRefreshRate;
    else
        minRefreshRate = MIN_REFRESH_RATE;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRate(void) {
    return refreshRate;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(uint32_t dmaRamToKeepFreeBytes) {
    cbInit(&dmaBuffer, ESP32_NUM_FRAME_BUFFERS);

    printf("Starting SmartMatrix DMA Mallocs\r\n");

    // TODO: malloc this buffer before other smaller buffers as this is (by far) the largest buffer to allocate?
    matrixUpdateFrames[0] = (frameStruct *)heap_caps_malloc(sizeof(frameStruct), MALLOC_CAP_DMA);
    assert(matrixUpdateFrames[0] != NULL);
    matrixUpdateFrames[1] = (frameStruct *)heap_caps_malloc(sizeof(frameStruct), MALLOC_CAP_DMA);
    assert(matrixUpdateFrames[1] != NULL);

    printf("sizeof framestruct: %08" PRIX32 "\r\n", (uint32_t)sizeof(frameStruct));
    show_esp32_dma_mem("DMA Memory Available before ptr1 alloc");
    printf("matrixUpdateFrames[0] pointer: %08" PRIX32 "\r\n", (uint32_t)matrixUpdateFrames[0]);
    show_esp32_dma_mem("DMA Memory Available before ptr2 alloc");
    printf("matrixUpdateFrames[1] pointer: %08" PRIX32 "\r\n", (uint32_t)matrixUpdateFrames[1]);

    printf("Frame Structs Allocated from Heap:\r\n");
    show_esp32_all_mem();

    printf("Allocating refresh buffer:\r\n");

    // setup debug output
#ifdef DEBUG_PINS_ENABLED
    gpio_pad_select_gpio(DEBUG_1_GPIO);
    gpio_set_direction(DEBUG_1_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DEBUG_1_GPIO, 1);
    gpio_set_level(DEBUG_1_GPIO, 0);

#ifdef DEBUG_2_GPIO
    gpio_pad_select_gpio(DEBUG_2_GPIO);
    gpio_set_direction(DEBUG_2_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(DEBUG_2_GPIO, 1);
    gpio_set_level(DEBUG_2_GPIO, 0);
#endif
#endif

    // calculate the lowest LSBMSB_TRANSITION_BIT value that will fit in memory
    int numDescriptorsPerRow;
    lsbMsbTransitionBit = 0;
    while(1) {
        numDescriptorsPerRow = 1;
        for(int i=lsbMsbTransitionBit + 1; i<COLOR_DEPTH_BITS; i++) {
            numDescriptorsPerRow += 1<<(i - lsbMsbTransitionBit - 1);
        }

        int ramrequired = numDescriptorsPerRow * MATRIX_SCAN_MOD * ESP32_NUM_FRAME_BUFFERS * sizeof(lldesc_t);
        int largestblockfree = heap_caps_get_largest_free_block(MALLOC_CAP_DMA);

        printf("lsbMsbTransitionBit of %d requires %d RAM, %d available, leaving %d free: \r\n", lsbMsbTransitionBit, ramrequired, largestblockfree, largestblockfree - ramrequired);

        if(largestblockfree > dmaRamToKeepFreeBytes && ramrequired < (largestblockfree - dmaRamToKeepFreeBytes))
            break;

        if(lsbMsbTransitionBit < COLOR_DEPTH_BITS - 1)
            lsbMsbTransitionBit++;
        else
            break;
    }

    if(numDescriptorsPerRow * MATRIX_SCAN_MOD * ESP32_NUM_FRAME_BUFFERS * sizeof(lldesc_t) > heap_caps_get_largest_free_block(MALLOC_CAP_DMA)){
        printf("not enough RAM for SmartMatrix descriptors\r\n");
        return;
    }

    printf("Raised lsbMsbTransitionBit to %d/%d to fit in RAM\r\n", lsbMsbTransitionBit, COLOR_DEPTH_BITS - 1);

    // calculate the lowest LSBMSB_TRANSITION_BIT value that will fit in memory that will meet or exceed the configured refresh rate
    while(1) {
        int psPerClock = 1000000000000UL/ESP32_I2S_CLOCK_SPEED;
        int nsPerLatch = ((PIXELS_PER_LATCH + CLKS_DURING_LATCH) * psPerClock) / 1000;
        //printf("ns per latch: %d: \r\n", nsPerLatch);        

        // add time to shift out LSBs + LSB-MSB transition bit - this ignores fractions...
        int nsPerRow = COLOR_DEPTH_BITS * nsPerLatch;

        // add time to shift out MSBs
        for(int i=lsbMsbTransitionBit + 1; i<COLOR_DEPTH_BITS; i++)
            nsPerRow += (1<<(i - lsbMsbTransitionBit - 1)) * (COLOR_DEPTH_BITS - i) * nsPerLatch;

        //printf("nsPerRow: %d: \r\n", nsPerRow);        

        int nsPerFrame = nsPerRow * MATRIX_SCAN_MOD;
        //printf("nsPerFrame: %d: \r\n", nsPerFrame);        

        int actualRefreshRate = 1000000000UL/(nsPerFrame);

        refreshRate = actualRefreshRate;

        printf("lsbMsbTransitionBit of %d gives %d Hz refresh, %d requested: \r\n", lsbMsbTransitionBit, actualRefreshRate, minRefreshRate);        

        if(actualRefreshRate >= minRefreshRate)
            break;

        if(lsbMsbTransitionBit < COLOR_DEPTH_BITS - 1)
            lsbMsbTransitionBit++;
        else
            break;
    }

    printf("Raised lsbMsbTransitionBit to %d/%d to meet minimum refresh rate\r\n", lsbMsbTransitionBit, COLOR_DEPTH_BITS - 1);

    // TODO: completely fill buffer with data before enabling DMA - can't do this now, lsbMsbTransition bit isn't set in the calc class - also this call will probably have no effect as matrixCalcDivider will skip the first call
    //matrixCalcCallback();

    // lsbMsbTransition Bit is now finalized - redo descriptor count in case it changed to hit min refresh rate
    numDescriptorsPerRow = 1;
    for(int i=lsbMsbTransitionBit + 1; i<COLOR_DEPTH_BITS; i++) {
        numDescriptorsPerRow += 1<<(i - lsbMsbTransitionBit - 1);
    }

    printf("Descriptors for lsbMsbTransitionBit %d/%d with %d rows require %d bytes of DMA RAM\r\n", lsbMsbTransitionBit, COLOR_DEPTH_BITS - 1, MATRIX_SCAN_MOD, 2 * numDescriptorsPerRow * MATRIX_SCAN_MOD * sizeof(lldesc_t));

    // malloc the DMA linked list descriptors that i2s_parallel will need
    int desccount = numDescriptorsPerRow * MATRIX_SCAN_MOD;
    lldesc_t * dmadesc_a = (lldesc_t *)heap_caps_malloc(desccount * sizeof(lldesc_t), MALLOC_CAP_DMA);
    if(!dmadesc_a) {
        printf("can't malloc dmadesc_a");
        return;
    }
    lldesc_t * dmadesc_b = (lldesc_t *)heap_caps_malloc(desccount * sizeof(lldesc_t), MALLOC_CAP_DMA);
    if(!dmadesc_b) {
        printf("can't malloc dmadesc_b");
        return;
    }

    printf("SmartMatrix Mallocs Complete\r\n");
    show_esp32_all_mem();

    lldesc_t *prevdmadesca = 0;
    lldesc_t *prevdmadescb = 0;
    int currentDescOffset = 0;

    // fill DMA linked lists for both frames
    for(int j=0; j<MATRIX_SCAN_MOD; j++) {
        // first set of data is LSB through MSB, single pass - all color bits are displayed once, which takes care of everything below and inlcluding LSBMSB_TRANSITION_BIT
        // TODO: size must be less than DMA_MAX - worst case for SmartMatrix Library: 16-bpp with 256 pixels per row would exceed this, need to break into two
        link_dma_desc(&dmadesc_a[currentDescOffset], prevdmadesca, matrixUpdateFrames[0]->rowdata[j].rowbits[0].data, sizeof(rowBitStruct) * COLOR_DEPTH_BITS);
        prevdmadesca = &dmadesc_a[currentDescOffset];
        link_dma_desc(&dmadesc_b[currentDescOffset], prevdmadescb, matrixUpdateFrames[1]->rowdata[j].rowbits[0].data, sizeof(rowBitStruct) * COLOR_DEPTH_BITS);
        prevdmadescb = &dmadesc_b[currentDescOffset];
        currentDescOffset++;
        //printf("row %d: \r\n", j);

        for(int i=lsbMsbTransitionBit + 1; i<COLOR_DEPTH_BITS; i++) {
            // binary time division setup: we need 2 of bit (LSBMSB_TRANSITION_BIT + 1) four of (LSBMSB_TRANSITION_BIT + 2), etc
            // because we sweep through to MSB each time, it divides the number of times we have to sweep in half (saving linked list RAM)
            // we need 2^(i - LSBMSB_TRANSITION_BIT - 1) == 1 << (i - LSBMSB_TRANSITION_BIT - 1) passes from i to MSB
            //printf("buffer %d: repeat %d times, size: %d, from %d - %d\r\n", nextBufdescIndex, 1<<(i - LSBMSB_TRANSITION_BIT - 1), (COLOR_DEPTH_BITS - i), i, COLOR_DEPTH_BITS-1);
            for(int k=0; k < 1<<(i - lsbMsbTransitionBit - 1); k++) {
                link_dma_desc(&dmadesc_a[currentDescOffset], prevdmadesca, matrixUpdateFrames[0]->rowdata[j].rowbits[i].data, sizeof(rowBitStruct) * (COLOR_DEPTH_BITS - i));
                prevdmadesca = &dmadesc_a[currentDescOffset];
                link_dma_desc(&dmadesc_b[currentDescOffset], prevdmadescb, matrixUpdateFrames[1]->rowdata[j].rowbits[i].data, sizeof(rowBitStruct) * (COLOR_DEPTH_BITS - i));
                prevdmadescb = &dmadesc_b[currentDescOffset];

                currentDescOffset++;
                //printf("i %d, j %d, k %d\r\n", i, j, k);
            }
        }
    }

    //End markers
    dmadesc_a[desccount-1].eof = 1;
    dmadesc_b[desccount-1].eof = 1;
    dmadesc_a[desccount-1].qe.stqe_next=(lldesc_t*)&dmadesc_a[0];
    dmadesc_b[desccount-1].qe.stqe_next=(lldesc_t*)&dmadesc_b[0];

    //printf("\n");

    // send FM6126A chipset reset sequence, which is ignored by other chipsets that don't need it
    // Thanks to Bob Davis: http://bobdavis321.blogspot.com/2019/02/p3-64x32-hub75e-led-matrix-panels-with.html
    if(optionFlags & SMARTMATRIX_OPTIONS_FM6126A_RESET_AT_START) { 
        int C12[16] = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
        int C13[16] = {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0};

        gpio_pad_select_gpio(CLK_PIN);
        gpio_set_direction(CLK_PIN, GPIO_MODE_OUTPUT);
#ifdef CLK_MANUAL_PIN        
        gpio_pad_select_gpio(CLK_MANUAL_PIN);
        gpio_set_direction(CLK_MANUAL_PIN, GPIO_MODE_OUTPUT);
#endif
        gpio_pad_select_gpio(LAT_PIN);
        gpio_set_direction(LAT_PIN, GPIO_MODE_OUTPUT);
        gpio_pad_select_gpio(OE_PIN);
        gpio_set_direction(OE_PIN, GPIO_MODE_OUTPUT);
        gpio_pad_select_gpio(R1_PIN);
        gpio_set_direction(R1_PIN, GPIO_MODE_OUTPUT);
        gpio_pad_select_gpio(G1_PIN);
        gpio_set_direction(G1_PIN, GPIO_MODE_OUTPUT);
        gpio_pad_select_gpio(B1_PIN);
        gpio_set_direction(B1_PIN, GPIO_MODE_OUTPUT);
        gpio_pad_select_gpio(R2_PIN);
        gpio_set_direction(R2_PIN, GPIO_MODE_OUTPUT);
        gpio_pad_select_gpio(G2_PIN);
        gpio_set_direction(G2_PIN, GPIO_MODE_OUTPUT);
        gpio_pad_select_gpio(B2_PIN);
        gpio_set_direction(B2_PIN, GPIO_MODE_OUTPUT);

#if (A_PIN >= 0)
        gpio_pad_select_gpio(A_PIN);
        gpio_set_direction(A_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(A_PIN, 1);
#endif
#if (B_PIN >= 0)
        gpio_pad_select_gpio(B_PIN);
        gpio_set_direction(B_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(B_PIN, 0);
#endif
#if (C_PIN >= 0)
        gpio_pad_select_gpio(C_PIN);
        gpio_set_direction(C_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(C_PIN, 0);
#endif
#if (D_PIN >= 0)
        gpio_pad_select_gpio(D_PIN);
        gpio_set_direction(D_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(D_PIN, 0);
#endif
#if (E_PIN >= 0)
        gpio_pad_select_gpio(E_PIN);
        gpio_set_direction(E_PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(E_PIN, 0);
#endif

        // keep display off
        gpio_set_level(OE_PIN, 1);

        // set CLK/LAT to idle state
        gpio_set_level(LAT_PIN, 0);
        gpio_set_level(CLK_PIN, 0);
#ifdef CLK_MANUAL_PIN        
        gpio_set_level(CLK_MANUAL_PIN, 0);
#endif

        // Send Data to control register 11
        for(int i=0; i<PIXELS_PER_LATCH; i++) {
            int y=i%16;
            gpio_set_level(R1_PIN, 0);
            gpio_set_level(G1_PIN, 0);
            gpio_set_level(B1_PIN, 0);
            gpio_set_level(R2_PIN, 0);
            gpio_set_level(G2_PIN, 0);
            gpio_set_level(B2_PIN, 0);

            if(C12[y] == 1) {
                gpio_set_level(R1_PIN, 1);
                gpio_set_level(G1_PIN, 1);
                gpio_set_level(B1_PIN, 1);
                gpio_set_level(R2_PIN, 1);
                gpio_set_level(G2_PIN, 1);
                gpio_set_level(B2_PIN, 1);                
            }

            delay(1);

            if(i > PIXELS_PER_LATCH-12)
                gpio_set_level(LAT_PIN, 1);
            else
                gpio_set_level(LAT_PIN, 0);

            delay(1);

#ifdef CLK_MANUAL_PIN        
            gpio_set_level(CLK_MANUAL_PIN, 1);
            gpio_set_level(CLK_MANUAL_PIN, 0);
#endif
            gpio_set_level(CLK_PIN, 1);
            delay(1);
            gpio_set_level(CLK_PIN, 0);
            delay(1);
        }

        gpio_set_level(LAT_PIN, 0);

        // Send Data to control register 12
        for(int i=0; i<PIXELS_PER_LATCH; i++) {
            int y=i%16;
            gpio_set_level(R1_PIN, 0);
            gpio_set_level(G1_PIN, 0);
            gpio_set_level(B1_PIN, 0);
            gpio_set_level(R2_PIN, 0);
            gpio_set_level(G2_PIN, 0);
            gpio_set_level(B2_PIN, 0);

            if(C13[y] == 1) {
                gpio_set_level(R1_PIN, 1);
                gpio_set_level(G1_PIN, 1);
                gpio_set_level(B1_PIN, 1);
                gpio_set_level(R2_PIN, 1);
                gpio_set_level(G2_PIN, 1);
                gpio_set_level(B2_PIN, 1);                
            }

            delay(1);

            if(i > PIXELS_PER_LATCH-13)
                gpio_set_level(LAT_PIN, 1);
            else
                gpio_set_level(LAT_PIN, 0);

            delay(1);

#ifdef CLK_MANUAL_PIN        
            gpio_set_level(CLK_MANUAL_PIN, 1);
            gpio_set_level(CLK_MANUAL_PIN, 0);
#endif
            gpio_set_level(CLK_PIN, 1);
            delay(1);
            gpio_set_level(CLK_PIN, 0);
            delay(1);
        }

        gpio_set_level(LAT_PIN, 0);

        // let changes settle before refreshing
        delay(1);
    }

#ifdef CLK_MANUAL_PIN
    // this pin can be manually toggled when the latch pin is high to send CLK pulses to the panel (normally latch blocks the clock signal on the ESP32 circuit).  Set idle to low so there's no extra CLK pulse when latch goes high   
    gpio_pad_select_gpio(CLK_MANUAL_PIN);
    gpio_set_direction(CLK_MANUAL_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(CLK_MANUAL_PIN, 0);
#endif

    i2s_parallel_config_t cfg={
        .gpio_bus={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, LAT_PIN, OE_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, -1, -1, -1},
        .gpio_clk=CLK_PIN,
        .clk_inversion=(optionFlags & SMARTMATRIX_OPTIONS_ESP32_INVERT_CLK),
        .clkspeed_hz=ESP32_I2S_CLOCK_SPEED,  // formula used is 80000000L/(cfg->clkspeed_hz + 1), must result in >=2.  Acceptable values 26.67MHz, 20MHz, 16MHz, 13.34MHz...
        .bits=MATRIX_I2S_MODE,
        .bufa=0,
        .bufb=0,
        .desccount_a=desccount,
        .desccount_b=desccount,
        .lldesc_a=dmadesc_a,
		.lldesc_b=dmadesc_b
    };

    //Setup I2S
    i2s_parallel_setup_without_malloc(&I2S1, &cfg);

    //printf("I2S setup done.\n");
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::markRefreshComplete(void) {
    if(!cbIsEmpty(&SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer))
        cbRead(&SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint8_t SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getLsbMsbTransitionBit(void) {
    return lsbMsbTransitionBit;
}
