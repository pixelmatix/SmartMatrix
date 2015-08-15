#include "SmartMatrix.h"

DMAChannel dmaOutputAddress(false);
DMAChannel dmaUpdateAddress(false);
DMAChannel dmaUpdateTimer(false);
DMAChannel dmaClockOutData(false);


CircularBuffer dmaBuffer;
