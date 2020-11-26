/*
 * SmartMatrix Library - Hardware-Specific Header File (for SmartMatrix Shield V4 with wire mod to connect Teensy 4)
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

// Hardware-specific header file to use the SmartLED Shield V4 with Teensy 4.0/4.1 without any adapter.
//   Other Teensy models are not supported. APA LED driving is not supported.
// The following hardware modifications are required:
// 1. Attach two wires to pins 32 and 33. For Teensy 4.0, this requires soldering to two of the pads on the
//    underside of the board. For Teensy 4.1, these pins are along the outer edges and are connected using header pins.
// 2. Insert the other ends of these two wires into the sockets in the SmartLED Shield to make the following connections:
//    2a. Connect pin 32 to pin 5.
//    2b. Connect pin 33 to pin 3.
// 3. Then make the following cross-connections using jumper wires inserted into the SmartLED Shield V4 sockets:
//    3a. Connect pin 10 to pin 14.
//    3b. Connect pin 12 to pin 2.
//    3c. Connect pin 11 to pin 20.
//    3d. Connect pin 9 to pin 21.
// Pins 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 20, 21, 32, and 33 are not available for other functions.

// Note: only one MatrixHardware_*.h file should be included per project

#ifndef MATRIX_HARDWARE_H
#define MATRIX_HARDWARE_H

#pragma message "MatrixHardware: Teensy 4 Wired to SmartLED Shield for Teensy 3 (V4)"

/* an advanced user may need to tweak these values */

// size of latch pulse - can be short for V4 shield which doesn't need to update ADDX lines during latch pulse
// 20 is minimum working value on DP5020B panel
// set to 100 for improved support with FM6126A panel
// don't exceed 150 to avoid interference between latch and data transfer
#define LATCH_TIMER_PULSE_WIDTH_NS 100

// max delay from rising edge of latch pulse to falling edge of first pixel clock
// increase this value if DMA use is delaying clock
// Measured 220 ns delay at 600 MHz clock speed, 160 ns at 816 MHz. Slower speeds are not supported.
// Using larger delay for safety.
#define LATCH_TO_CLK_DELAY_NS 400

// Pixel clock frequency is generated using the 480 MHz PLL3 clock and a divide-by-n counter. Frequency is independent of CPU clock speed.
// Must increment divider value by 2 (division ratio is always even)
// Minimum tested working value is 20 (corresponding to 24 MHz clock frequency) on DP5020B panel
// Using value of 26 (corresponding to 18.462 MHz clock frequency) to improve stability and reduce glitching
// Larger values may be more stable, but will decrease maximum refresh rate
#define FLEXIO_CLOCK_DIVIDER 26

// Amount of time required to transfer 32 pixels
// Adding 200 ns overhead time to improve stability
#define PANEL_32_PIXELDATA_TRANSFER_MAXIMUM_NS  ((32*FLEXIO_CLOCK_DIVIDER*1000/480) + 200)

/* this section describes how the microcontroller is attached to the display */

// these defines control the color channel assignments - if your panel has non-standard RGB order, swap signals here
#define R_0_SIGNAL    FLEXIO_PIN_R0_TEENSY_PIN
#define G_0_SIGNAL    FLEXIO_PIN_G0_TEENSY_PIN
#define B_0_SIGNAL    FLEXIO_PIN_B0_TEENSY_PIN
#define R_1_SIGNAL    FLEXIO_PIN_R1_TEENSY_PIN
#define G_1_SIGNAL    FLEXIO_PIN_G1_TEENSY_PIN
#define B_1_SIGNAL    FLEXIO_PIN_B1_TEENSY_PIN

/************ The below definitions are unlikely to be useful if changed **************/

// active pin number definitions
// data bit order is calculated in setup using the pin number definitions and color channel assignments
// these pin definitions are also manually used to reset FM6126A panels
#define FLEXPWM_PIN_OE_TEENSY_PIN       4
#define FLEXPWM_PIN_LATCH_TEENSY_PIN    33    // jumper wire to Teensy Pin 3
#define FLEXIO_PIN_CLK_TEENSY_PIN       10    // jumper wire to Teensy Pin 14
#define FLEXIO_PIN_CLK_TEENSY_PIN_ALT   7     // jumper wire to Teensy Pin 14
#define FLEXIO_PIN_B0_TEENSY_PIN        6
#define FLEXIO_PIN_R0_TEENSY_PIN        12    // jumper wire to Teensy Pin 2
#define FLEXIO_PIN_R1_TEENSY_PIN        9     // jumper wire to Teensy Pin 21
#define FLEXIO_PIN_G0_TEENSY_PIN        32    // jumper wire to Teensy Pin 5
#define FLEXIO_PIN_G1_TEENSY_PIN        8
#define FLEXIO_PIN_B1_TEENSY_PIN        11    // jumper wire to Teensy Pin 20
// To use the alternate clock pin, you must set flag SMARTMATRIX_OPTIONS_T4_CLK_PIN_ALT in kMatrixOptions

// APA102 driving is not supported with this hardware
#define SMARTLED_APA_ENABLED_BY_DEFAULT true
#define SMARTLED_APA_ENABLE_PIN         0xFF
#define FLEXIO_PIN_APA102_CLK           0xFF
#define FLEXIO_PIN_APA102_DAT           0xFF

// the following pins cannot be used if the jumper wires are connected
#define T4_RESERVED_PINS_SMARTLED_SHIELD_V4
#define T4_RESERVED_LATCH_TEENSY_PIN    3
#define T4_RESERVED_CLK_TEENSY_PIN      14
#define T4_RESERVED_R0_TEENSY_PIN       2
#define T4_RESERVED_R1_TEENSY_PIN       21
#define T4_RESERVED_G0_TEENSY_PIN       5
#define T4_RESERVED_B1_TEENSY_PIN       20

#define ADDX_0_SIGNAL   FLEXIO_PIN_R0_TEENSY_PIN
#define ADDX_1_SIGNAL   FLEXIO_PIN_G0_TEENSY_PIN
#define ADDX_2_SIGNAL   FLEXIO_PIN_B0_TEENSY_PIN
#define ADDX_3_SIGNAL   FLEXIO_PIN_R1_TEENSY_PIN
#define ADDX_4_SIGNAL   FLEXIO_PIN_G1_TEENSY_PIN

#else
    #pragma GCC error "Multiple MatrixHardware*.h files included"
#endif
