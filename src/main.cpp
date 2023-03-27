#include <Arduino.h>

#include <dispatch.h>
#include <serial_task.h>
#include <bluetooth_task.h>
#include <radio_task.h>

#define pro_cpu 0
#define app_cpu 1


char my_message[80];
uint32_t temp_code = 432221;

char received_code_form_device_string[10];
uint32_t received_code_from_device = 0;

// void echoDeviceSentEventsToNotifyDevice(){
//     notificationToSend.isIndicate = true;
//     char notify_message[80]; 

// }

void bleEventHandlerCallback(uint8_t eventID, char value[18]){
    if (eventID == WRITE_EVENT){
        sprintf(my_message, "Got %s", value);
        // notifyBleDevices(true, my_message);
        writeToSerial(my_message);
        // Send Event Recieved From BLE Device
        if (strncmp(value, "Send", 4) == 0){
            memset(received_code_form_device_string, 0, 10);
            strcpy(received_code_form_device_string, value + 5);
            // writeToSerial(received_code_form_device_string);
            received_code_from_device = atoi(received_code_form_device_string);
            sendCodeToRadio(received_code_from_device);
        } else if (strncmp(value, "Learn", 5) == 0){
            // writeToSerial("Learn Command Initiaing");
            putRadioIntoLearning();
        } else if (strncmp(value, "Cancel", 6) == 0){
            // What to do?
        }
    }
}


char status_message[30];
void radioEventHandlerCallback(uint8_t status, uint32_t value){
    memset(status_message, 0, 30);
    switch (status){
        case RADIO_IDLE_STATUS:
            writeToSerial("Radio Is Idle");
            break;
        case RADIO_SENDING_CODE_STATUS:
            // writeToSerial("Radio Is Sending:");
            // writeToSerial(itoa(value, my_message, 10));
            break;
        case RADIO_CODE_SENT_STATUS:
            strcpy(status_message, "Radio Sent:");
            writeToSerial(status_message);
            memset(status_message, 0, 30);
            writeToSerial(itoa(value, status_message, 10));
            memset(status_message, 0, 30);
            sprintf(status_message, "Sent Code: %d", value);
            Serial.println(status_message);
            notifyBleDevices(true, status_message);
            break;
        case RADIO_LEARNING_CODE_STATUS:
            strcpy(status_message, "Radio is learning");
            writeToSerial(status_message);
            notifyBleDevices(true, status_message);
            break;
        case RADIO_CODE_LEARNED_STATUS:
            // Serial.println("RADIO LEARNED!!!");
            temp_code = value;
            // strcpy(status_message, "Radio Successfully Learned Code:");
            // writeToSerial(status_message);
            // notifyBleDevices(true, status_message);
            // writeToSerial(itoa(value, my_message, 10));
            sprintf(status_message, "Learned: %d", value);
            writeToSerial(status_message);
            notifyBleDevices(true, status_message);
            // putRadioIntoIdle();
            break;
        case RADIO_CODE_LEARN_CANCELED_STATUS:
            Serial.println("Why Cancelled?");
            // strcpy(status_message, "Radio Learning Cancelled");
            // writeToSerial(status_message);
            // notifyBleDevices(true, status_message);
            // putRadioIntoIdle();
            break;
    }
}

void setup(){

    xTaskCreatePinnedToCore(
        handleSerialTask,
        "Handle Serial Task",
        2048,
        NULL,
        4,
        getSerialTaskHandle(),
        app_cpu
    );

    // Wait for Serial interface to load
    vTaskDelay(1000 / portTICK_RATE_MS);
    writeToSerial("Testing Serial");

    xTaskCreatePinnedToCore(
        bleMainTask,
        "Handle Blutooth Task",
        5000,
        NULL,
        3,
        getBLETaskHandle(),
        pro_cpu
    );

    // Wait for Bluetooth interface to load
    vTaskDelay(1000 / portTICK_RATE_MS);

    xTaskCreatePinnedToCore(
        radioMainTask,
        "Handle Radio Task",
        5000,
        NULL,
        4,
        getRadioTaskHandle(),
        app_cpu
    );

    // Wait for Radio interface to load
    vTaskDelay(1000 / portTICK_RATE_MS);


    // notificationToSend.isIndicate = true;
    // char notify_message[80]; 

    // while (true){
    //     if (xQueueReceive(getBleDeviceSentEventsQueue(), &lastEvent, 1000 / portMAX_DELAY) == pdTRUE){
    //         // writeToSerial(lastEvent.eventValue);
    //         if (lastEvent.eventID == WRITE_EVENT){
    //             writeToSerial(lastEvent.eventValue);
    //             sprintf(notify_message, "Got %s", lastEvent.eventValue);
    //             strncpy(notificationToSend.value, notify_message, 18);
    //             xQueueSend(getBleNotifyToDeviceQueue(), &notificationToSend, 1000 / portMAX_DELAY);
    //             if (strncmp(lastEvent.eventValue, "open", 4) == 0){
    //                 radioCommand.command = RADIO_SEND_COMMAND;
    //                 radioCommand.value = 444333;
    //                 xQueueSend(getRadioCommandQueue(), &radioCommand, portMAX_DELAY);
    //             }
    //             else if (strncmp(lastEvent.eventValue, "learn", 5) == 0){
    //                 radioCommand.command = RADIO_LEARN_COMMAND;
    //                 xQueueSend(getRadioCommandQueue(), &radioCommand, portMAX_DELAY);
    //             }
    //         }
    //         else {
    //             char print_message[2];
    //             writeToSerial(itoa(lastEvent.eventID, print_message, 10));
    //         }
    //     }
    // }


    start_dispatch();

    writeToSerial("Dispatch Started");

    addCallbackForBleDevicesEvents(bleEventHandlerCallback);
    addCallbackForRadioStatus(radioEventHandlerCallback);
    // while (true)
    // {
    //     ii++;
    //     sprintf(message, "Hello %i", ii);
    //     notifyBleDevices(true, message);
    //     vTaskDelay(1000 / portTICK_RATE_MS);
    // }
    




    // Maybe delete the starting task instead of delaying it indefinitely?
    vTaskDelay(portMAX_DELAY);

}

void loop(){

}