#include <serial_task.h>

TaskHandle_t *printToSerialTaskHandle = NULL;
QueueHandle_t serialWriteQueue = NULL;

char message[serialWriteMaxMessageLength];
void handleSerialTask(void *parameters){
    serialWriteQueue = xQueueCreate(serialWriteQueueLength, serialWriteMaxMessageLength);
    Serial.begin(115200);
    Serial.println("Serial Initialized");
    while (true){
        if (xQueueReceive(serialWriteQueue, &message, portMAX_DELAY) == pdTRUE){
            Serial.println(message);
        }
    }
}

TaskHandle_t *getSerialTaskHandle(){
    return printToSerialTaskHandle;
}

QueueHandle_t getSerialWriteQueue(){
    return serialWriteQueue;
}