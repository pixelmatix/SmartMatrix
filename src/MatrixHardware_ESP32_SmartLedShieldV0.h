/*
 * SmartMatrix Library - Hardware-Specific Header File (ESP32 SmartLED Shield V0 ESP32)
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

 // Note: only one MatrixHardware_*.h file should be included per project

#ifndef MATRIX_HARDWARE_H
#define MATRIX_HARDWARE_H
// formula used is 80000000L/(cfg->clkspeed_hz + 1), must result in >=2.  Acceptable values 26.67MHz, 20MHz, 16MHz, 13.34MHz...
#define ESP32_I2S_CLOCK_SPEED (20000000UL)

#define ESP32_FORUM_PINOUT              0
#define ESP32_FORUM_PINOUT_WITH_LATCH   1
#define SMARTLED_SHIELD_V0_PINOUT       2
#define ESP32_JC_RIBBON_PINOUT          3
#define HUB75_ADAPTER_PINOUT            4
#define HUB75_ADAPTER_LATCH_BREADBOARD_PINOUT            5
#define HUB75_ADAPTER_V0_THT_PINOUT     6
#define HUB75_ADAPTER_V0_SMT_PINOUT     7
#define ESP32_JC_RIBBON_PINOUT_WEMOS    8
#define HUB75_ADAPTER_LITE_V0_PINOUT    9
#define ESP32_RGB64x32MatrixPanel_I2S_DMA_DEFAULT    10

#ifdef GPIOPINOUT
#pragma GCC error "GPIOPINOUT previously set!"
#endif

#define GPIOPINOUT SMARTLED_SHIELD_V0_PINOUT

#pragma message "MatrixHardware: ESP32 SmartLED shield V0 pinout"

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
#define MATRIX_I2S_MODE I2S_PARALLEL_BITS_8
#define MATRIX_DATA_STORAGE_TYPE uint8_t
#define CLKS_DURING_LATCH   4

#undef BIT_A
#undef BIT_B
#undef BIT_C
#undef BIT_D
#undef BIT_E

#define R1_PIN  GPIO_NUM_4
#define G1_PIN  GPIO_NUM_21
#define B1_PIN  GPIO_NUM_22
#define R2_PIN  GPIO_NUM_2
#define G2_PIN  GPIO_NUM_25
#define B2_PIN  GPIO_NUM_0

#define A_PIN   -1
#define B_PIN   -1
#define C_PIN   -1
#define D_PIN   -1
#define E_PIN   -1

#define LAT_PIN GPIO_NUM_27
#define OE_PIN  GPIO_NUM_32

#define CLK_PIN GPIO_NUM_26

#else
    #pragma GCC error "Multiple MatrixHardware*.h files included"
#endif
