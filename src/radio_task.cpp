#include <radio_task.h>
// #include <dispatch.h>
#include <RCSwitch.h>

static uint32_t learnedCode = 0; 
static uint32_t codeToSend = 0; 

#define RADIO_GPIO_PIN 17
#define RADIO_TRANSMIT_PIN 16

QueueHandle_t radioCommandQueue;
QueueHandle_t radioStatusQueue;
uint8_t queueLength = 1;
TaskHandle_t *radioTaskHandle = NULL;

RadioCommandStruct lastCommand;
RadioStatusStruct lastStatus;
// RCSwitch rcswitch;
//     rcswitch = RCSwitch();
RCSwitch rcswitch;

TaskHandle_t *radioLearnCodeTaskHandle = NULL;

int learnHappened = 0;
int cancelLearn = 0;

void sendCode(uint32_t code) {
    lastStatus.status = RADIO_SENDING_CODE_STATUS;
    lastStatus.payload = code;
    xQueueSend(radioStatusQueue, &lastStatus, portMAX_DELAY);

    rcswitch.enableTransmit(RADIO_TRANSMIT_PIN);
    // Config more about protocl to send, bit length, ...

    rcswitch.send(code, 24);

    rcswitch.disableTransmit();

    // rcswitch.enableReceive(RADIO_GPIO_PIN);
    // rcswitch.resetAvailable();

    lastStatus.status = RADIO_CODE_SENT_STATUS;
    lastStatus.payload = code;
    xQueueSend(radioStatusQueue, &lastStatus, portMAX_DELAY);
}


void cancelLearnCode(){
    cancelLearn = 1;
}

void idleRadio(){
    rcswitch.disableReceive();
    rcswitch.disableTransmit();
    lastStatus.status = RADIO_IDLE_STATUS;
    lastStatus.payload = 0;
    xQueueSend(radioStatusQueue, &lastStatus, portMAX_DELAY);
}

// static TimerHandle_t learningCodeTimeoutTimer = NULL;
// void learningCodeTimeoutCallback(TimerHandle_t xtimer){
//     Serial.println("Timer Triggered");
//     cancelLearn = 1;
// }

int numberOfEqualConsequentCodes = 1;
void setLearnedCode(uint32_t code){
    if (code == learnedCode){
        numberOfEqualConsequentCodes++;
    }
    else {
        numberOfEqualConsequentCodes = 1;
        learnedCode = code;
    }

}

SemaphoreHandle_t isSendingSemaphore;

void constantRCSwitchReadingTask(void *parameters){
    while (true){
        xSemaphoreTake(isSendingSemaphore, portMAX_DELAY);
        if (rcswitch.available()){
            setLearnedCode(rcswitch.getReceivedValue());
            Serial.println(learnedCode);
            Serial.println(rcswitch.getReceivedBitlength());
            Serial.println(rcswitch.getReceivedProtocol());
            rcswitch.resetAvailable();
        }
        xSemaphoreGive(isSendingSemaphore);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

}


void radioMainTask(void *parameters){
    isSendingSemaphore = xSemaphoreCreateBinary();

    rcswitch = RCSwitch();
    rcswitch.enableReceive(RADIO_GPIO_PIN);
    
    xSemaphoreGive(isSendingSemaphore);

    xTaskCreatePinnedToCore(
        constantRCSwitchReadingTask,
        "Constant Task",
        1024,
        NULL,
        2,
        NULL,
        APP_CPU_NUM
    );

    // learningCodeTimeoutTimer = xTimerCreate(
    //     "Learn Code Timeout",
    //     10000 / portTICK_PERIOD_MS,
    //     pdFALSE,
    //     (void *) 0,
    //     learningCodeTimeoutCallback);
    radioCommandQueue = xQueueCreate(queueLength, sizeof(RadioCommandStruct));
    radioStatusQueue = xQueueCreate(queueLength, sizeof(RadioStatusStruct));
    while (true){
        if(xQueueReceive(radioCommandQueue, &lastCommand, portMAX_DELAY) == pdTRUE){
            if (&lastCommand != NULL){
                switch (lastCommand.command){
                    case RADIO_SEND_COMMAND:
                        xSemaphoreTake(isSendingSemaphore, portMAX_DELAY);
                        codeToSend = lastCommand.value;
                        rcswitch.disableReceive();
                        sendCode(codeToSend);
                        pinMode(RADIO_GPIO_PIN, INPUT);
                        rcswitch.enableReceive(RADIO_GPIO_PIN);
                        rcswitch.resetAvailable();
                        xSemaphoreGive(isSendingSemaphore);

                        break;
                    case RADIO_LEARN_COMMAND:
                        // Change Serial To writeToSerial
                        Serial.println("Learn Command Received");
                        while(numberOfEqualConsequentCodes < 3)
                        {
                            vTaskDelay(100 / portTICK_PERIOD_MS);
                        }
                        Serial.println("Three Consequent Codes Equal");
                        numberOfEqualConsequentCodes = 1;
                        RadioStatusStruct newStatus;
                        newStatus.status = RADIO_CODE_LEARNED_STATUS;
                        newStatus.payload = learnedCode;
                        // lastStatus.status = RADIO_CODE_LEARNED_STATUS;
                        // lastStatus.payload = learnedCode;
                        if (xQueueSend(radioStatusQueue, &newStatus, portMAX_DELAY) == pdTRUE) {
                            Serial.print("Queue Sent Success");
                        }
                        break;
                    case RADIO_CANCEL_LEARN_COMMAND:
                        cancelLearnCode();
                        break;
                    case RADIO_IDLE_COMMAND:
                        idleRadio();
                        break;
                }
            }
        }
    }
}

QueueHandle_t getRadioCommandQueue(){
    return radioCommandQueue;
}

QueueHandle_t getRadioStatusQueue(){
    return radioStatusQueue;
}

TaskHandle_t *getRadioTaskHandle(){
    return radioTaskHandle;
}