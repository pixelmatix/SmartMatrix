/*
 * SmartMatrix Library - Hardware-Specific Header File (ESP32 RGB64x32MatrixPanel-I2S-DMA Library default pinout)
 *
 * Copyright (c) 2020 Beaudoin (Pixelmatix)
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

 // Note: only one MatrixHardware_*.h file should be included per project

#ifndef MATRIX_HARDWARE_H
#define MATRIX_HARDWARE_H

// formula used is 80000000L/(cfg->clkspeed_hz + 1), must result in >=2.  Acceptable values 26.67MHz, 20MHz, 16MHz, 13.34MHz...
#define ESP32_I2S_CLOCK_SPEED (20000000UL)

#define ESP32_RGB64x32MatrixPanel_I2S_DMA_DEFAULT    10

#ifdef GPIOPINOUT
#pragma GCC error "GPIOPINOUT previously set!"
#endif

#define GPIOPINOUT HUB75_ADAPTER_LITE_V0_PINOUT

#pragma message "MatrixHardware: ESP32-RGB64x32MatrixPanel-I2S-DMA default pinout"

//Upper half RGB
#define BIT_R1  (1<<0)   
#define BIT_G1  (1<<1)   
#define BIT_B1  (1<<2)   
//Lower half RGB
#define BIT_R2  (1<<3)   
#define BIT_G2  (1<<4)   
#define BIT_B2  (1<<5)   

// Control Signals
#define BIT_LAT (1<<6) 
#define BIT_OE  (1<<7)  

#define BIT_A (1<<8)    
#define BIT_B (1<<9)    
#define BIT_C (1<<10)   
#define BIT_D (1<<11)   
#define BIT_E (1<<12)   

// ADDX is output on RGB pins and stored in external latch (need multiple of 32-bits for full data struct, so pad 2 CLKs to 4 here)
#define MATRIX_I2S_MODE I2S_PARALLEL_BITS_16
#define MATRIX_DATA_STORAGE_TYPE uint16_t
#define CLKS_DURING_LATCH   0

#define R1_PIN  GPIO_NUM_25
#define G1_PIN  GPIO_NUM_26
#define B1_PIN  GPIO_NUM_27
#define R2_PIN  GPIO_NUM_14
#define G2_PIN  GPIO_NUM_12
#define B2_PIN  GPIO_NUM_13

#define A_PIN   GPIO_NUM_23
#define B_PIN   GPIO_NUM_19
#define C_PIN   GPIO_NUM_5
#define D_PIN   GPIO_NUM_17
#define E_PIN   GPIO_NUM_32

#define LAT_PIN GPIO_NUM_4
#define OE_PIN  GPIO_NUM_15

#define CLK_PIN GPIO_NUM_16

//#define DEBUG_PINS_ENABLED
#define DEBUG_1_GPIO    GPIO_NUM_18
#define DEBUG_2_GPIO    GPIO_NUM_23

#else
    #pragma GCC error "Multiple MatrixHardware*.h files included"
#endif
