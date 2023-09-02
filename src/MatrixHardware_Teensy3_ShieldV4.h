/*
 * SmartMatrix Library - Hardware-Specific Header File (for SmartMatrix Shield for Teensy 3 V4)
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

#ifndef SM_INTERNAL
#pragma message "MatrixHardware: SmartLED Shield for Teensy 3 (V4)"
#endif

#define DMA_UPDATES_PER_CLOCK           2
#define ADDX_UPDATE_ON_DATA_PINS
#define ADDX_UPDATE_BEFORE_LATCH_BYTES  1

/* an advanced user may need to tweak these values */

// size of latch pulse - can be short for V4 which doesn't need to update ADDX lines during latch pulse
#define LATCH_TIMER_PULSE_WIDTH_NS  100

// max delay from rising edge of latch pulse to falling edge of clock
// increase this value if DMA use is delaying clock
// using largest delay seen at slowest supported clock speed (48MHz) 1400ns, saw 916ns at 96MHz
#define LATCH_TO_CLK_DELAY_NS       1400

// measured <3400ns to transfer 32 pixels at 96MHz, <6600ns to transfer 32 pixels at 48MHz
// for now, until DMA sharing complications (brought to light by Teensy 3.6 SDIO) can be worked out, enable DMA Bandwidth Control, which approximately doubles this estimated time
#define PANEL_32_PIXELDATA_TRANSFER_MAXIMUM_NS  (uint32_t)((2 * 3400 * 96000000.0) / F_CPU)

/* this section describes how the microcontroller is attached to the display */

// change for SmartMatrix Shield V4: G2 moves from Teensy pin 7 (D2) to 8 (D3)

// these defines map the HUB75 signal to PORTD signals - if your panel has non-standard RGB order, swap signals here
#define BIT_0_SIGNAL    hub75_r0
#define BIT_1_SIGNAL    hub75_clk
#define BIT_2_SIGNAL    pad
#define BIT_3_SIGNAL    hub75_g1
#define BIT_4_SIGNAL    hub75_b0
#define BIT_5_SIGNAL    hub75_b1
#define BIT_6_SIGNAL    hub75_r1
#define BIT_7_SIGNAL    hub75_g0

//#define DEBUG_PINS_ENABLED
#define DEBUG_PIN_1 16
#define DEBUG_PIN_2 18
#define DEBUG_PIN_3 19

/************ The below definitions are unlikely to be useful if changed **************/

// these pin definitions are used to set GPIO to output, and are manually used to reset FM6126A panels - swapping RGB pins here has no effect on refreshing the panel
#define GPIO_PIN_CLK_TEENSY_PIN     14
#define GPIO_PIN_B0_TEENSY_PIN      6
#define GPIO_PIN_R0_TEENSY_PIN      2
#define GPIO_PIN_R1_TEENSY_PIN      21
#define GPIO_PIN_G0_TEENSY_PIN      5
#define GPIO_PIN_G1_TEENSY_PIN      8
#define GPIO_PIN_B1_TEENSY_PIN      20

#define GPIO_PIN_LATCH_TEENSY_PIN   3
#define GPIO_PIN_OE_TEENSY_PIN      4

#define SMARTLED_APA_ENABLED_BY_DEFAULT false
#define SMARTLED_APA_ENABLE_PIN     17
#define SMARTLED_APA_CLK_PIN        13
#define SMARTLED_APA_DAT_PIN        7


#define BIT_0_ADDX_SIGNAL   hub75_addx0
#define BIT_1_ADDX_SIGNAL   pad0
#define BIT_2_ADDX_SIGNAL   pad1
#define BIT_3_ADDX_SIGNAL   hub75_addx4
#define BIT_4_ADDX_SIGNAL   hub75_addx2
#define BIT_5_ADDX_SIGNAL   pad2
#define BIT_6_ADDX_SIGNAL   hub75_addx3
#define BIT_7_ADDX_SIGNAL   hub75_addx1


#define GPIO_WORD_ORDER_8BIT BIT_0_SIGNAL:1, BIT_1_SIGNAL:1, BIT_2_SIGNAL:1, BIT_3_SIGNAL:1, BIT_4_SIGNAL:1, BIT_5_SIGNAL:1, BIT_6_SIGNAL:1, BIT_7_SIGNAL:1
#define GPIO_WORD_ORDER_ADDX_8BIT BIT_0_ADDX_SIGNAL:1, BIT_1_ADDX_SIGNAL:1, BIT_2_ADDX_SIGNAL:1, BIT_3_ADDX_SIGNAL:1, BIT_4_ADDX_SIGNAL:1, BIT_5_ADDX_SIGNAL:1, BIT_6_ADDX_SIGNAL:1, BIT_7_ADDX_SIGNAL:1


// output latch signal on pin 3 (PORTA.12)
#define ENABLE_LATCH_PWM_OUTPUT() {                                     \
        CORE_PIN3_CONFIG |= PORT_PCR_MUX(3) | PORT_PCR_DSE | PORT_PCR_SRE;  \
    }

// output OE signal on pin 4 (PORTA.13)
#define ENABLE_OE_PWM_OUTPUT() {                                        \
        CORE_PIN4_CONFIG = PORT_PCR_MUX(3) | PORT_PCR_DSE | PORT_PCR_SRE;   \
    }

// pin 3 (PORTA.12) triggers based on latch signal, on rising edge
#define ENABLE_LATCH_RISING_EDGE_GPIO_INT() {              \
        CORE_PIN3_CONFIG |= PORT_PCR_MUX(1) | PORT_PCR_IRQC(1); \
    }

// unused for SmartMatrix Shield V4
#define ENABLE_LATCH_FALLING_EDGE_GPIO_INT() {              \
    }

#define DMAMUX_SOURCE_LATCH_FALLING_EDGE     DMAMUX_SOURCE_PORTA

#else
    #pragma GCC error "Multiple MatrixHardware*.h files included"
#endif
