/*
 * SmartMatrix Library - Hardware-Specific Header File
 *
 * Copyright (c) 2014 Louis Beaudoin (Pixelmatix)
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

// basic display size (32x32 1/16 refresh and 16x32 1/8 refresh display supported)
// to use multiple matrices in a row, set MATRIX_WIDTH to the total width
#define MATRIX_HEIGHT       32
#define MATRIX_WIDTH        32
#define PANEL_HEIGHT        32
#define PANEL_WIDTH         32
#define DRAWING_HEIGHT      32
#define DRAWING_WIDTH       32
#define PANELS_PER_ROW      MATRIX_WIDTH / PANEL_WIDTH

// an advanced user may need to tweak these values
#define MATRIX_REFRESH_RATE         120
// only 24-bit color supported
#define COLOR_DEPTH_RGB             24
// DMA_BUFFER_NUMBER_OF_ROWS = the size of the buffer that DMA pulls from to refresh the display
// must be minimum 2 rows so one can be updated while the otehr is refreshed
// increase beyond two to give more time for the update routine to complete
// (increase this number if non-DMA interrupts are causing display problems)
#define DMA_BUFFER_NUMBER_OF_ROWS   4
// size of latch pulse - all address updates must fit inside high portion of latch pulse
// increase this value if DMA use is causing address updates to take longer
#define LATCH_TIMER_PULSE_WIDTH_NS  438
// set this by triggering scope on latch rising edge, and with persistence enabled,
// look for the last clock pulse after the latch.  set the min block period to be beyond this last pulse
// default (10us) is a generous minimum that should work with all Teensy 3.x devices at 48MHz and above
#define MIN_BLOCK_PERIOD_NS     10000

// this section describes how the microcontroller is attached to the display

// defines data bit order from bit 0-7, four times to fit in uint32_t
#define GPIO_WORD_ORDER p0r1:1, p0clk:1, p0g2:1, p0pad:1, p0b1:1, p0b2:1, p0r2:1, p0g1:1, \
    p1r1:1, p1clk:1, p1g2:1, p1pad:1, p1b1:1, p1b2:1, p1r2:1, p1g1:1, \
    p2r1:1, p2clk:1, p2g2:1, p2pad:1, p2b1:1, p2b2:1, p2r2:1, p2g1:1, \
    p3r1:1, p3clk:1, p3g2:1, p3pad:1, p3b1:1, p3b2:1, p3r2:1, p3g1:1

#define GPIO_PIN_CLK_TEENSY_PIN     14
#define GPIO_PIN_B0_TEENSY_PIN      6
#define GPIO_PIN_R0_TEENSY_PIN      2
#define GPIO_PIN_R1_TEENSY_PIN      21
#define GPIO_PIN_G0_TEENSY_PIN      5
#define GPIO_PIN_G1_TEENSY_PIN      7
#define GPIO_PIN_B1_TEENSY_PIN      20

#define ADDX_PIN_0  3
#define ADDX_PIN_1  4
#define ADDX_PIN_2  1
#define ADDX_PIN_3  2
#define ADDX_PIN_MASK   ((1 << ADDX_PIN_0) | (1 << ADDX_PIN_1) | (1 << ADDX_PIN_2) | (1 << ADDX_PIN_3))

#define ADDX_TEENSY_PIN_0   9
#define ADDX_TEENSY_PIN_1   10
#define ADDX_TEENSY_PIN_2   22
#define ADDX_TEENSY_PIN_3   23

#define ADDX_GPIO_SET_REGISTER      GPIOC_PSOR
#define ADDX_GPIO_CLEAR_REGISTER    GPIOC_PCOR

// output latch signal on two pins, to trigger two different GPIO port interrupts
#define ENABLE_LATCH_PWM_OUTPUT() {                                     \
        CORE_PIN3_CONFIG |= PORT_PCR_MUX(3) | PORT_PCR_DSE | PORT_PCR_SRE;  \
    }

#define ENABLE_OE_PWM_OUTPUT() {                                        \
        CORE_PIN4_CONFIG = PORT_PCR_MUX(3) | PORT_PCR_DSE | PORT_PCR_SRE;   \
    }

// pin 3 (PORT A) triggers based on latch signal, on rising edge
#define ENABLE_LATCH_RISING_EDGE_GPIO_INT() {       \
        CORE_PIN3_CONFIG |= PORT_PCR_IRQC(1);           \
    }

#define DMAMUX_SOURCE_LATCH_RISING_EDGE     DMAMUX_SOURCE_PORTA

// pin 8 (PORT D3) is set to input, and triggers based on latch signal, on falling edge
#define ENABLE_LATCH_FALLING_EDGE_GPIO_INT() {              \
        CORE_PIN8_CONFIG |= PORT_PCR_MUX(1) | PORT_PCR_IRQC(2); \
    }

#define DMAMUX_SOURCE_LATCH_FALLING_EDGE     DMAMUX_SOURCE_PORTD

#endif
