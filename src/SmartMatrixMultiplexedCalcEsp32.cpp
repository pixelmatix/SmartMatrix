#include "SmartMatrix3.h"

#if defined(ESP32)

SemaphoreHandle_t calcTaskSemaphore;

void IRAM_ATTR matrixCalculationsSignal(void) {
    static BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    // Unblock the task by releasing the semaphore.
    xSemaphoreGiveFromISR(calcTaskSemaphore, &xHigherPriorityTaskWoken );
    if( xHigherPriorityTaskWoken != pdFALSE )
    {
        // We can force a context switch here.  Context switching from an
        // ISR uses port specific syntax.  Check the demo task for your port
        // to find the syntax required.
    }
}
#endif