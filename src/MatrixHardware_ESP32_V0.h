/*
 * SmartMatrix Library - Hardware-Specific Header File (ESP32 pinout collection)
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
#define ELECTRODRAGON_PINOUT    11

#ifndef GPIOPINOUT
#define GPIOPINOUT ELECTRODRAGON_PINOUT
#endif
//#define GPIOPINOUT ESP32_FORUM_PINOUT_WITH_LATCH // note this mode is untested as of 2018-05-17 - not being used anymore now that SmartMatrix Shield is available
//#define GPIOPINOUT SMARTLED_SHIELD_V0_PINOUT

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


#if (GPIOPINOUT == ESP32_JC_RIBBON_PINOUT)
    #pragma message "MatrixHardware: Jason Coon ESP32 NodeMCU shield wiring"
// This pinout takes a ribbon cable and flattens it, pin order is 1, 9, 2, 10 ...
// it connects to https://www.tindie.com/products/jasoncoon/16-output-nodemcu-esp32-wifi-ble-led-controller/
// *** WARNING, I cut the trace on Jason's board that went to pin 3, and patched a wire
// to pin 27 so that I can use RX/TX serial debugging ****
// That shield's pinout is this for the output of the level shifters:
// 23, 22, 27 (was 3), 21, 19, 18, 5, 17,    16, 4, 0, 2, 15, 14, 12, 13

    // ADDX is output directly using GPIO
    #define CLKS_DURING_LATCH   0 
    #define MATRIX_I2S_MODE I2S_PARALLEL_BITS_16
    #define MATRIX_DATA_STORAGE_TYPE uint16_t

    /*
    HUB 75 pinout
    01 02 B0
    03 04 Gnd
    05 06 G1
    07 08 E

    09 10 B
    11 12 D
    13 14 STB/Latch
    15 16 Gnd

                        ESP32 pin / comment
    1	R0	23	Red Data (columns 1-16)
    2	G0	22   	Green Data (columns 1-16)

    3	B0	27	(was 3) Blue Data (columns 1-16)
    4	GND	21/GND	Ground

    5	R1	19	Red Data (columns 17-32)
    6	G1	18 	Green Data (columns 17-32)

    7	B1	5    	Blue Data (columns 17-32)
    8	E	17/TX2	Demux Input E for 64x64 panels

    9	A	16/RX2	Demux Input A0
    10	B	4	Demux Input A1
    
    11	C	0/Boot	Demux Input A2
    12	D	2 	Demux Input E1, E3 (32x32 panels only)

    13	CLK	15 	LED Drivers' Clock
    14	STB	14 	LED Drivers' Latch
    
    15	OE	12	LED Drivers' Output Enable
    16	GND	13/GND	Ground
    */ 
    #define R1_PIN  GPIO_NUM_23
    #define G1_PIN  GPIO_NUM_22
    #define B1_PIN  GPIO_NUM_27
    #define R2_PIN  GPIO_NUM_19
    #define G2_PIN  GPIO_NUM_18
    #define B2_PIN  GPIO_NUM_5

    #define A_PIN   GPIO_NUM_16
    #define B_PIN   GPIO_NUM_4
    #define C_PIN   GPIO_NUM_0
    #define D_PIN   GPIO_NUM_2
    #define E_PIN   GPIO_NUM_17

    #define CLK_PIN GPIO_NUM_15
    #define LAT_PIN GPIO_NUM_14
    #define OE_PIN  GPIO_NUM_12

#elif (GPIOPINOUT == ESP32_JC_RIBBON_PINOUT_WEMOS)
    #pragma message "MatrixHardware: Jason Coon ESP32 Wemos/Lolin shield wiring"
// This pinout takes a ribbon cable and flattens it, pin order is 1, 9, 2, 10 ...
// it connects to https://www.tindie.com/products/jasoncoon/16-output-wemos-d32-wifi-ble-led-controller/
// *** WARNING, I cut the trace on Jason's board that went to pin 3, and patched a wire
// to pin 27 so that I can use RX/TX serial debugging ****
// That shield's pinout is this for the output of the level shifters:
// NodeMCU:    23, 22, 27 (was 3), 21,   19, 18, 5, 17,   16, 4,  0,  2,   15, 14, 12, 13
// WemosLolin: 23, 22, 27 (was 3), 21,   19, 18, 5, 4,     0, 2, 15, 25,   26, 14, 12, 13

    // ADDX is output directly using GPIO
    #define CLKS_DURING_LATCH   0 
    #define MATRIX_I2S_MODE I2S_PARALLEL_BITS_16
    #define MATRIX_DATA_STORAGE_TYPE uint16_t

    /*
    HUB 75 pinout
    01 02 B0
    03 04 Gnd
    05 06 G1
    07 08 E

    09 10 B
    11 12 D
    13 14 STB/Latch
    15 16 Gnd

                        ESP32 pin / comment
    1   R0  23  Red Data (columns 1-16)
    2   G0  22      Green Data (columns 1-16)

    3   B0  27  (was 3) Blue Data (columns 1-16)
    4   GND 21/GND  Ground

    5   R1  19  Red Data (columns 17-32)
    6   G1  18  Green Data (columns 17-32)

    7   B1  5       Blue Data (columns 17-32)
    8   E   17/TX2  Demux Input E for 64x64 panels

    9   A   16/RX2  Demux Input A0
    10  B   4   Demux Input A1
    
    11  C   0/Boot  Demux Input A2
    12  D   2   Demux Input E1, E3 (32x32 panels only)

    13  CLK 15  LED Drivers' Clock
    14  STB 14  LED Drivers' Latch
    
    15  OE  12  LED Drivers' Output Enable
    16  GND 13/GND  Ground
    */ 
    #define R1_PIN  GPIO_NUM_23
    #define G1_PIN  GPIO_NUM_22
    #define B1_PIN  GPIO_NUM_27
    #define R2_PIN  GPIO_NUM_19
    #define G2_PIN  GPIO_NUM_18
    #define B2_PIN  GPIO_NUM_5

    #define A_PIN   GPIO_NUM_0
    #define B_PIN   GPIO_NUM_2
    #define C_PIN   GPIO_NUM_15
    #define D_PIN   GPIO_NUM_25
    #define E_PIN   GPIO_NUM_4

    #define CLK_PIN GPIO_NUM_26
    #define LAT_PIN GPIO_NUM_14
    #define OE_PIN  GPIO_NUM_12

#elif (GPIOPINOUT == ESP32_FORUM_PINOUT)

    #pragma message "MatrixHardware: ESP32 forum wiring"

    // ADDX is output directly using GPIO
    #define CLKS_DURING_LATCH   0 
    #define MATRIX_I2S_MODE I2S_PARALLEL_BITS_16
    #define MATRIX_DATA_STORAGE_TYPE uint16_t

    /*
    HUB 75
    01 02 B0
    03 04 Gnd
    05 06 G1
    07 08 E

    09 10 B
    11 12 D
    13 14 STB/Latch
    15 16 Gnd
                        ESP32 pin / comment
    1	R0	2	Red Data (columns 1-16)
    2	G0	15	Green Data (columns 1-16)

    3	B0	4	Blue Data (columns 1-16)
    4	GND	GND	Ground
    
    5	R1	16/RX2	Red Data (columns 17-32)
    6	G1	27	Green Data (columns 17-32)
    
    7	B1	17/TX2	Blue Data (columns 17-32)
    8	E	12	Demux Input E for 64x64 panels
   
    9	A	5	Demux Input A0
    10	B	18	Demux Input A1

    11	C	19	Demux Input A2
    12	D	21	Demux Input E1, E3 (32x32 panels only)
    
    13	CLK	22	LED Drivers' Clock
    14	STB	26	LED Drivers' Latch
    
    15	OE	25	LED Drivers' Output Enable
    16	GND	GND	Ground
    */ 
    #define R1_PIN  GPIO_NUM_2
    #define G1_PIN  GPIO_NUM_15
    #define B1_PIN  GPIO_NUM_4
    #define R2_PIN  GPIO_NUM_16
    #define G2_PIN  GPIO_NUM_27
    #define B2_PIN  GPIO_NUM_17

    #define A_PIN   GPIO_NUM_5
    #define B_PIN   GPIO_NUM_18
    #define C_PIN   GPIO_NUM_19
    #define D_PIN   GPIO_NUM_21
    #define E_PIN   GPIO_NUM_12
    #define LAT_PIN GPIO_NUM_26
    #define OE_PIN  GPIO_NUM_25

    #define CLK_PIN GPIO_NUM_22

#elif (GPIOPINOUT == ELECTRODRAGON_PINOUT)


    // https://www.electrodragon.com/product/mono-color-hub12-matrix-panel-drive-board-esp32/
    // https://w2.electrodragon.com/Board-dat/IDD/IDD1023-dat/IDD1023-dat.md
    // https://github.com/Edragon/Arduino-ESP32/tree/master/libraries/DMD32-main
    // https://github.com/Edragon/Arduino-ESP32/blob/master/libraries/DMD32-main/connection.png

    #pragma message "MatrixHardware: Electrodragon Mono Color HUB12 Matrix Panel Drive Board ESP32 wiring"

    // ADDX is output directly using GPIO
    #define CLKS_DURING_LATCH   0
    #define MATRIX_I2S_MODE I2S_PARALLEL_BITS_16
    #define MATRIX_DATA_STORAGE_TYPE uint16_t

    /*
    HUB 75
    01 02 B0
    03 04 Gnd
    05 06 G1
    07 08 E

    09 10 B
    11 12 D
    13 14 STB/Latch
    15 16 Gnd
                        ESP32 pin / comment
    1	R0	23	Red Data (columns 1-16)

    4	GND	GND	Ground

    9	A	19	Demux Input A0
    10	B	21	Demux Input A1

    13	CLK	18	LED Drivers' Clock
    14	STB	2	LED Drivers' Latch

    15	OE	22	LED Drivers' Output Enable
    16	GND	GND	Ground
    */
    #define R1_PIN  GPIO_NUM_23
    #define G1_PIN  GPIO_NUM_NC
    #define B1_PIN  GPIO_NUM_NC
    #define R2_PIN  GPIO_NUM_NC
    #define G2_PIN  GPIO_NUM_NC
    #define B2_PIN  GPIO_NUM_NC

    #define A_PIN   GPIO_NUM_19
    #define B_PIN   GPIO_NUM_21
    #define C_PIN   GPIO_NUM_NC
    #define D_PIN   GPIO_NUM_NC
    #define E_PIN   GPIO_NUM_NC
    #define LAT_PIN GPIO_NUM_2
    #define OE_PIN  GPIO_NUM_22

    #define CLK_PIN GPIO_NUM_18

#elif (GPIOPINOUT == HUB75_ADAPTER_PINOUT)

    #pragma message "MatrixHardware: Hub75 Adapter Pinout"

    // ADDX is output directly using GPIO
    #define CLKS_DURING_LATCH   0 
    #define MATRIX_I2S_MODE I2S_PARALLEL_BITS_16
    #define MATRIX_DATA_STORAGE_TYPE uint16_t

    /*
    HUB 75
    01 02 B0
    03 04 Gnd
    05 06 G1
    07 08 E

    09 10 B
    11 12 D
    13 14 STB/Latch
    15 16 Gnd
                        ESP32 pin / comment
    1   R0  2   Red Data (columns 1-16)
    2   G0  15  Green Data (columns 1-16)

    3   B0  4   Blue Data (columns 1-16)
    4   GND GND Ground
    
    5   R1  16/RX2  Red Data (columns 17-32)
    6   G1  27  Green Data (columns 17-32)
    
    7   B1  17/TX2  Blue Data (columns 17-32)
    8   E   12  Demux Input E for 64x64 panels
   
    9   A   5   Demux Input A0
    10  B   18  Demux Input A1

    11  C   19  Demux Input A2
    12  D   21  Demux Input E1, E3 (32x32 panels only)
    
    13  CLK 22  LED Drivers' Clock
    14  STB 26  LED Drivers' Latch
    
    15  OE  25  LED Drivers' Output Enable
    16  GND GND Ground
    */ 
    #define R1_PIN  GPIO_NUM_12
    #define G1_PIN  GPIO_NUM_14
    #define B1_PIN  GPIO_NUM_33
    #define R2_PIN  GPIO_NUM_25
    #define G2_PIN  GPIO_NUM_9
    #define B2_PIN  GPIO_NUM_18

    #define A_PIN   GPIO_NUM_19
    #define B_PIN   GPIO_NUM_22
    #define C_PIN   GPIO_NUM_21
    #define D_PIN   GPIO_NUM_4
    #define E_PIN   GPIO_NUM_26
    #define LAT_PIN GPIO_NUM_15
    #define OE_PIN  GPIO_NUM_13

    #define CLK_PIN GPIO_NUM_2

#elif (GPIOPINOUT == ESP32_FORUM_PINOUT_WITH_LATCH)
    #pragma message "MatrixHardware: ESP32 forum wiring with external 74AHCT373 latch circuit - note untested since 2018, may be broken"

    // Note: this is untested since 2018, may be broken

    // ADDX is output on RGB pins and stored in external 74AHCT373 latch (need multiple of 32-bits for full data struct, so pad 2 CLKs to 4 here)
    #define MATRIX_I2S_MODE I2S_PARALLEL_BITS_8
    #define MATRIX_DATA_STORAGE_TYPE uint8_t
    #define CLKS_DURING_LATCH   4

    #define R1_PIN  GPIO_NUM_2
    #define G1_PIN  GPIO_NUM_15
    #define B1_PIN  GPIO_NUM_4
    #define R2_PIN  GPIO_NUM_16
    #define G2_PIN  GPIO_NUM_27
    #define B2_PIN  GPIO_NUM_17

    #define A_PIN   GPIO_NUM_5
    #define B_PIN   GPIO_NUM_18
    #define C_PIN   GPIO_NUM_19
    #define D_PIN   GPIO_NUM_21
    #define E_PIN   GPIO_NUM_12
    #define LAT_PIN GPIO_NUM_26
    #define OE_PIN  GPIO_NUM_25

    #define CLK_PIN GPIO_NUM_22

#elif (GPIOPINOUT == SMARTLED_SHIELD_V0_PINOUT)

    #pragma message "MatrixHardware: ESP32 SmartLED shield V0 pinout"

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

    #define A_PIN   GPIO_NUM_NC
    #define B_PIN   GPIO_NUM_NC
    #define C_PIN   GPIO_NUM_NC
    #define D_PIN   GPIO_NUM_NC
    #define E_PIN   GPIO_NUM_NC

    #define LAT_PIN GPIO_NUM_27
    #define OE_PIN  GPIO_NUM_32

    #define CLK_PIN GPIO_NUM_26

#elif (GPIOPINOUT == HUB75_ADAPTER_LATCH_BREADBOARD_PINOUT)

    #pragma message "MatrixHardware: HUB75 Adapter Latch Breadboard pinout"

    // ADDX is output on RGB pins and stored in external latch (need multiple of 32-bits for full data struct, so pad 2 CLKs to 4 here)
    #define MATRIX_I2S_MODE I2S_PARALLEL_BITS_8
    #define MATRIX_DATA_STORAGE_TYPE uint8_t
    #define CLKS_DURING_LATCH   4

    #undef BIT_A
    #undef BIT_B
    #undef BIT_C
    #undef BIT_D
    #undef BIT_E

    #define R1_PIN  GPIO_NUM_12
    #define G1_PIN  GPIO_NUM_2
    #define B1_PIN  GPIO_NUM_15
    #define R2_PIN  GPIO_NUM_26
    #define G2_PIN  GPIO_NUM_25
    #define B2_PIN  GPIO_NUM_21

    #define A_PIN   GPIO_NUM_NC
    #define B_PIN   GPIO_NUM_NC
    #define C_PIN   GPIO_NUM_NC
    #define D_PIN   GPIO_NUM_NC
    #define E_PIN   GPIO_NUM_NC

    #define LAT_PIN GPIO_NUM_22
    #define OE_PIN  GPIO_NUM_19

    #define CLK_PIN GPIO_NUM_4

#elif (GPIOPINOUT == HUB75_ADAPTER_V0_THT_PINOUT)

    #pragma message "MatrixHardware: HUB75 Adapter V0 THT pinout"

    // ADDX is output on RGB pins and stored in external latch (need multiple of 32-bits for full data struct, so pad 2 CLKs to 4 here)
    #define MATRIX_I2S_MODE I2S_PARALLEL_BITS_8
    #define MATRIX_DATA_STORAGE_TYPE uint8_t
    #define CLKS_DURING_LATCH   4

    #undef BIT_A
    #undef BIT_B
    #undef BIT_C
    #undef BIT_D
    #undef BIT_E

    #define R1_PIN  GPIO_NUM_12
    #define G1_PIN  GPIO_NUM_2
    #define B1_PIN  GPIO_NUM_15
    #define R2_PIN  GPIO_NUM_26
    #define G2_PIN  GPIO_NUM_25
    #define B2_PIN  GPIO_NUM_21

    #define A_PIN   GPIO_NUM_NC
    #define B_PIN   GPIO_NUM_NC
    #define C_PIN   GPIO_NUM_NC
    #define D_PIN   GPIO_NUM_NC
    #define E_PIN   GPIO_NUM_NC

    #define LAT_PIN GPIO_NUM_22
    #define OE_PIN  GPIO_NUM_19

    #define CLK_PIN GPIO_NUM_4

    // this pin can be manually toggled when the latch pin is high to send CLK pulses to the panel (noramlly latch blocks clock)
    #define CLK_MANUAL_PIN GPIO_NUM_18

#elif (GPIOPINOUT == HUB75_ADAPTER_V0_SMT_PINOUT)

    #pragma message "MatrixHardware: HUB75 Adapter V0 SMT pinout"

    // ADDX is output on RGB pins and stored in external latch (need multiple of 32-bits for full data struct, so pad 2 CLKs to 4 here)
    #define MATRIX_I2S_MODE I2S_PARALLEL_BITS_8
    #define MATRIX_DATA_STORAGE_TYPE uint8_t
    #define CLKS_DURING_LATCH   4

    #undef BIT_A
    #undef BIT_B
    #undef BIT_C
    #undef BIT_D
    #undef BIT_E

    #define R1_PIN  GPIO_NUM_12
    #define G1_PIN  GPIO_NUM_27
    #define B1_PIN  GPIO_NUM_26
    #define R2_PIN  GPIO_NUM_2
    #define G2_PIN  GPIO_NUM_25
    #define B2_PIN  GPIO_NUM_15

    #define A_PIN   GPIO_NUM_NC
    #define B_PIN   GPIO_NUM_NC
    #define C_PIN   GPIO_NUM_NC
    #define D_PIN   GPIO_NUM_NC
    #define E_PIN   GPIO_NUM_NC

    #define LAT_PIN GPIO_NUM_10
    #define OE_PIN  GPIO_NUM_9

    #define CLK_PIN GPIO_NUM_4

    // this pin can be manually toggled when the latch pin is high to send CLK pulses to the panel (noramlly latch blocks clock)
    #define CLK_MANUAL_PIN GPIO_NUM_22

#endif

//#define DEBUG_PINS_ENABLED
#define DEBUG_1_GPIO    GPIO_NUM_13
//#define DEBUG_2_GPIO    GPIO_NUM_12

#else
    #pragma GCC error "Multiple MatrixHardware*.h files included"
#endif
