#include <Arduino.h>

#include "BLEDevice.h"
#include <BLEClient.h>
#include <BLEScan.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>
#include <BLE2902.h>

#include <dispatch.h>

#include <bluetooth_task.h>


#define GARAGE_DOOR_SERVICE_UUID        "4acfc201-1fb5-459e-8fcc-c5c9c331914b"

#define STATUS_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define COMMAND_CHARACTERISTIC_UUID2 "dfb5483e-36e1-4688-b7f5-8807361b26a8"

static BLEAdvertising *advertising;
static BLEServer *server = NULL;

static BLEService *garage_service = NULL; 
static BLECharacteristic *input_characteristic = NULL;
static BLECharacteristic *output_characteristic = NULL;

// BLE Task:
static TaskHandle_t *bleMainTaskHandle = NULL;

// BLE Queues & Their Objects
struct BleDeviceSentEvent bleEvent;
struct BleNotifyDevice bleNotificationToDevice;
QueueHandle_t bleDeviceSentEventsQueue = NULL;
QueueHandle_t bleNotifyDeviceQueue = NULL;
static const uint8_t bleQueuesLength = 2;



static void _sendDeviceSentEventsToQueue(uint8_t bleEventID, const char * eventValue){
    if (&bleEvent != NULL){
        bleEvent.eventID = bleEventID;
        strncpy(bleEvent.eventValue, eventValue, 18);
        xQueueSend(bleDeviceSentEventsQueue, &bleEvent, 10 / portTICK_RATE_MS);
    }
}

class DeviceSentEventsCallbacks: public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic *characteristic){
        _sendDeviceSentEventsToQueue(WRITE_EVENT, characteristic->getValue().c_str());
    }
    void onRead(BLECharacteristic *characteristic){
        _sendDeviceSentEventsToQueue(READ_EVENT, characteristic->getValue().c_str());
    }
    void onNotify(BLECharacteristic *characteristic){
        _sendDeviceSentEventsToQueue(NOTIFY_EVENT, characteristic->getValue().c_str());
    }
    void onStatus(BLECharacteristic *characteristic){
        _sendDeviceSentEventsToQueue(STATUS_EVENT, characteristic->getValue().c_str());
    }
};


static void initializeBLE(){
    BLEDevice::init("My Garage Opener");
    server = BLEDevice::createServer();

    garage_service = server->createService(GARAGE_DOOR_SERVICE_UUID);

    // This is the characteristic that the connected device writes to in order to communicate with
    // current program. We gave read property so that the connected device be able to query the last
    // sent command.
    input_characteristic = garage_service->createCharacteristic(
        COMMAND_CHARACTERISTIC_UUID2,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    input_characteristic->addDescriptor(new BLE2902());
    input_characteristic->setValue("Initialized");
    input_characteristic->setCallbacks(new DeviceSentEventsCallbacks());

    // This is the characteristic that the connected device listens on (notify, indicate or even busy read).
    // This is mainly used for reporting status of the current program to connected device(s).
    output_characteristic = garage_service->createCharacteristic(
        STATUS_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_INDICATE |
        BLECharacteristic::PROPERTY_NOTIFY);

    output_characteristic->addDescriptor(new BLE2902());
    output_characteristic->setValue("Initialized");

    // TODO: Dig more into advertising and its paramerters
    advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(GARAGE_DOOR_SERVICE_UUID);
    advertising->setScanResponse(false);
    advertising->setMinPreferred(0x0);
}

void bleMainTask(void *parameters){
    bleDeviceSentEventsQueue = xQueueCreate(bleQueuesLength, sizeof(BleDeviceSentEvent));
    bleNotifyDeviceQueue = xQueueCreate(bleQueuesLength, sizeof(BleNotifyDevice));

    initializeBLE();

    garage_service->start();
    BLEDevice::startAdvertising();

    writeToSerial("Started Advertising");

    // TODO: I think we need this, but don't know the minimum / maximum delay possible.
    // Here we listen on bleNotifyDeviceQueue for any notification. Whenever one enters queue, we send it 
    // to the connected device(s) using notify mechanism.
    while(true){
        if (xQueueReceive(bleNotifyDeviceQueue, &bleNotificationToDevice, 10 / portTICK_RATE_MS) == pdTRUE){
            char *notificationValue = bleNotificationToDevice.value;
            output_characteristic->setValue(notificationValue);
            output_characteristic->notify();
        }
        vTaskDelay(10 / portTICK_RATE_MS);
    }

    // Maintain Advertisement After A Period Of Time. How?
}

TaskHandle_t *getBLETaskHandle(){
    return bleMainTaskHandle;
}

QueueHandle_t getBleDeviceSentEventsQueue(){
    return bleDeviceSentEventsQueue;
}

QueueHandle_t getBleNotifyToDeviceQueue(){
    return bleNotifyDeviceQueue;
}