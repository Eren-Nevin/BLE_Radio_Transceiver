#include <Arduino.h>

const uint8_t serialWriteQueueLength = 5;
const uint8_t serialWriteMaxMessageLength = 80;


TaskHandle_t *getSerialTaskHandle();
QueueHandle_t getSerialWriteQueue();

void handleSerialTask(void *parameters);