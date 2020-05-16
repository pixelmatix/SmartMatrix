#ifndef SMARTMATRIXREFRESHT4_H
#define SMARTMATRIXREFRESHT4_H

#include <FlexIO_t4.h> // requires FlexIO_t4 library from https://github.com/KurtE/FlexIO_t4

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
class SmartMatrixRefreshT4 {
  public:
    struct __attribute__((packed, aligned(2))) timerpair {
      uint16_t timer_oe;
      uint16_t timer_period;
    };

    struct __attribute__((packed, aligned(4))) rowBitStruct {
      uint16_t data[PIXELS_PER_LATCH];
      uint32_t rowAddress;
      timerpair timerValues __attribute__((aligned(2)));
    };

    struct rowDataStruct {
      rowBitStruct rowbits[refreshDepth / COLOR_CHANNELS_PER_PIXEL];
    };

    // struct to store bit offsets based on FlexIO hardware pin numbers
    struct flexPinConfigStruct {
      uint8_t r0;
      uint8_t g0;
      uint8_t b0;
      uint8_t r1;
      uint8_t g1;
      uint8_t b1;
    };

    typedef void (*matrix_underrun_callback)(void);
    typedef void (*matrix_calc_callback)(bool initial);

    // init
    SmartMatrixRefreshT4(uint8_t bufferrows, volatile rowDataStruct * rowDataBuf);
    static void begin(void);

    // refresh API
    static volatile rowDataStruct * getNextRowBufferPtr(void);
    static void writeRowBuffer(uint8_t currentRow);
    static void recoverFromDmaUnderrun(void);
    static bool isRowBufferFree(void);
    static void setRefreshRate(uint16_t newRefreshRate);
    static void setBrightness(uint8_t newBrightness);
    static void setMatrixCalculationsCallback(matrix_calc_callback f);
    static void setMatrixUnderrunCallback(matrix_underrun_callback f);
    static const flexPinConfigStruct & getFlexPinConfig(void);
    static void setRowAddress(unsigned int row);

  private:
    // enable ISR access to private member variables
    template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, unsigned char optionFlags1>
    friend void rowShiftCompleteISR(void);
    template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, unsigned char optionFlags1>
    friend void rowCalculationISR(void);

    // init helper function called by begin()
    static void hardwareSetup(void);

    // configuration helper function
    static void calculateTimerLUT(void);

    static int dimmingFactor;
    static const int dimmingMaximum = 255;
    static uint16_t rowBitStructBytesToShift;
    static uint16_t refreshRate;
    static uint8_t dmaBufferNumRows;
    static volatile rowDataStruct * matrixUpdateRows;

    static timerpair timerLUT[LATCHES_PER_ROW];
    static timerpair timerPairIdle;
    static matrix_calc_callback matrixCalcCallback;
    static matrix_underrun_callback matrixUnderrunCallback;

    static CircularBuffer_SM dmaBuffer;

    static IMXRT_FLEXIO_t *flexIO;
    static IMXRT_FLEXPWM_t * flexpwm;
    static uint8_t submodule;
    static volatile uint8_t enablerSourceByte;
    static flexPinConfigStruct flexPinConfig;
};

#endif
