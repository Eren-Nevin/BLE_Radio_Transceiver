
#include <dispatch.h>
#include <serial_task.h>
#include <bluetooth_task.h>
#include <radio_task.h>

#define pro_cpu 0
#define app_cpu 1

struct BleDeviceSentEvent lastBleEvent;
struct BleNotifyDevice notificationToSend;
struct BleCallback {
    bool set = false;
    void (*callback)(uint8_t eventID, char value[18]);
};
BleCallback bleCallbacks[10];

struct RadioCommandStruct lastRadioCommand;
struct RadioStatusStruct lastRadioStatus;
struct RadioCallback {
    bool set = false;
    void (*callback)(uint8_t status, uint32_t value);
};
RadioCallback radioCallbacks[10];

void radioListenTask(void *parameters);
void bleListenTask(void *parameters);

// Only call this after the corrosponding tasks are run
void start_dispatch(){

    // Maybe add task handlers

    xTaskCreatePinnedToCore(
        bleListenTask,
        "bleListenTask",
        5000,
        NULL,
        3,
        NULL,
        app_cpu
    );
    vTaskDelay(100 / portTICK_RATE_MS);
    xTaskCreatePinnedToCore(
        radioListenTask,
        "radioListenTask",
        5000,
        NULL,
        3,
        NULL,
        app_cpu
    );
    vTaskDelay(100 / portTICK_RATE_MS);

}

// Serial

char passed_message[serialWriteMaxMessageLength];

void _sendMessageBufferToSerialQueue(){
    xQueueSend(getSerialWriteQueue(), passed_message, portMAX_DELAY);
}

void writeToSerial(char *message_param){
    strncpy(passed_message, message_param, 80);
    _sendMessageBufferToSerialQueue();
}

void writeToSerial(const char *message_param){
    strncpy(passed_message, message_param, 80);
    _sendMessageBufferToSerialQueue();
}

void writeToSerial(std::string message_param){
    strncpy(passed_message, message_param.c_str(), 80);
    _sendMessageBufferToSerialQueue();
}


// Only call these after start() is called.
// Bluetooth

// Listener task is listening for ble and radio queues 

void bleListenTask(void *parameters){
    while (true){
        if (xQueueReceive(getBleDeviceSentEventsQueue(), &lastBleEvent, portMAX_DELAY) == pdTRUE){
            for (int i=0; i<10; i++){
                if (bleCallbacks[i].set == true){
                    bleCallbacks[i].callback(lastBleEvent.eventID, lastBleEvent.eventValue);
                }
            }
        }
    }
}

void notifyBleDevices(bool isIndicate, char *payload) {
    notificationToSend.isIndicate = isIndicate;
    strncpy(notificationToSend.value, payload, 18);
    xQueueSend(getBleNotifyToDeviceQueue(), &notificationToSend, portMAX_DELAY);
}

int8_t addCallbackForBleDevicesEvents(void callback(uint8_t eventID, char value[18])){
    for (uint8_t i=0; i<10; i++){
        if (bleCallbacks[i].set == false){
            bleCallbacks[i].set = true;
            bleCallbacks[i].callback = callback;
            return i;
        }
    }
    return -1;
}

bool removeCallbackForBleDevicesEvents(uint8_t callback_index){
    if (bleCallbacks[callback_index].set == true){
        bleCallbacks[callback_index].set = false;
        bleCallbacks[callback_index].callback = NULL;
        return true;
    }
    else {
        return false;
    }

}



// Radio

void radioListenTask(void *parameters){
    while (true){
        if (xQueueReceive(getRadioStatusQueue(), &lastRadioStatus, portMAX_DELAY) == pdTRUE){
            for (int i=0; i<10; i++){
                if (radioCallbacks[i].set == true){
                    radioCallbacks[i].callback(lastRadioStatus.status, lastRadioStatus.payload);
                }
            }
        }
    }
}

void sendCodeToRadio(uint32_t code){
    lastRadioCommand.command = RADIO_SEND_COMMAND;
    lastRadioCommand.value = code;
    xQueueSend(getRadioCommandQueue(), &lastRadioCommand, portMAX_DELAY);
}

void putRadioIntoLearning(){
    lastRadioCommand.command = RADIO_LEARN_COMMAND;
    lastRadioCommand.value = 0;
    xQueueSend(getRadioCommandQueue(), &lastRadioCommand, portMAX_DELAY);
}

void cancelRadioLearning(){
    lastRadioCommand.command = RADIO_CANCEL_LEARN_COMMAND;
    lastRadioCommand.value = 0;
    xQueueSend(getRadioCommandQueue(), &lastRadioCommand, portMAX_DELAY);
}

void putRadioIntoIdle(){
    lastRadioCommand.command = RADIO_IDLE_COMMAND;
    lastRadioCommand.value = 0;
    xQueueSend(getRadioCommandQueue(), &lastRadioCommand, portMAX_DELAY);
}

int8_t addCallbackForRadioStatus(void callback(uint8_t status, uint32_t value)){
    for (uint8_t i=0; i<10; i++){
        if (radioCallbacks[i].set == false){
            radioCallbacks[i].set = true;
            radioCallbacks[i].callback = callback;
            return i;
        }
    }
    return -1;
}

bool removeCallbackForRadioDevicesEvents(uint8_t callback_index){
    if (radioCallbacks[callback_index].set == true){
        radioCallbacks[callback_index].set = false;
        radioCallbacks[callback_index].callback = NULL;
        return true;
    }
    else {
        return false;
    }

}


