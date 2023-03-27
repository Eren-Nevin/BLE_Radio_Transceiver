#ifndef Arduino_h
#include <Arduino.h>
#endif

TaskHandle_t *getRadioTaskHandle();
void radioMainTask(void *parameters);

QueueHandle_t getRadioCommandQueue();
QueueHandle_t getRadioStatusQueue();

#define RADIO_SEND_COMMAND 0
#define RADIO_LEARN_COMMAND 1
#define RADIO_CANCEL_LEARN_COMMAND 2
#define RADIO_IDLE_COMMAND 3

#define RADIO_IDLE_STATUS 0
#define RADIO_SENDING_CODE_STATUS 1
#define RADIO_CODE_SENT_STATUS 2
#define RADIO_LEARNING_CODE_STATUS 3
#define RADIO_CODE_LEARNED_STATUS 4
#define RADIO_CODE_LEARN_CANCELED_STATUS 5

// Change the value to cover both protocol and the actual code itself.
struct RadioCommandStruct {
    uint8_t command;
    uint32_t value;
};

struct RadioStatusStruct {
    uint8_t status;
    uint32_t payload;
};
