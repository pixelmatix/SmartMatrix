#ifndef SMARTMATRIXCALCT4_H
#define SMARTMATRIXCALCT4_H

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
class SmartMatrix3 {
    public:
        typedef typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct rowDataStruct;

        // init
        SmartMatrix3(uint8_t bufferrows, volatile rowDataStruct * rowDataBuf);
        void begin(void);
        void addLayer(SM_Layer * newlayer);

        // configuration
        void setRotation(rotationDegrees newrotation);
        void setBrightness(uint8_t newBrightness);
        void setRefreshRate(uint16_t newRefreshRate);

        // get info
        uint16_t getScreenWidth(void) const;
        uint16_t getScreenHeight(void) const;
        uint16_t getRefreshRate(void);
        bool getdmaBufferUnderrunFlag(void);
        bool getRefreshRateLoweredFlag(void);

        // debug
        int countFPS(void);

        // functions called by ISR
        static void matrixCalculations(bool initial);
        static void dmaBufferUnderrunCallback(void);

    private:
        static SM_Layer * baseLayer;

        // functions for refreshing
        static void loadMatrixBuffers(unsigned int currentRow);
        static void loadMatrixBuffers48(volatile rowDataStruct * currentRowDataPtr, unsigned int currentRow);
        static void resetMultiRowRefreshMapPosition(void);
        static void resetMultiRowRefreshMapPositionPixelGroupToStartOfRow(void);
        static void advanceMultiRowRefreshMapToNextRow(void);
        static void advanceMultiRowRefreshMapToNextPixelGroup(void);
        static int getMultiRowRefreshRowOffset(void);
        static int getMultiRowRefreshNumPixelsToMap(void);
        static int getMultiRowRefreshPixelGroupOffset(void);

        // configuration
        static volatile bool brightnessChange;
        static volatile bool rotationChange;
        static volatile bool dmaBufferUnderrun;
        static uint8_t brightness;
        static rotationDegrees rotation;
        static uint16_t calc_refreshRate;
        static bool dmaBufferUnderrunSinceLastCheck;
        static bool refreshRateLowered;
        static bool refreshRateChanged;

        static int multiRowRefresh_mapIndex_CurrentRowGroups;
        static int multiRowRefresh_mapIndex_CurrentPixelGroup;
        static int multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped;
        static int multiRowRefresh_NumPanelsAlreadyMapped;
};

#endif
