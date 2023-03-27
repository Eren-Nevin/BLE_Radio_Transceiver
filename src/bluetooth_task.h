TaskHandle_t *getBLETaskHandle();
void bleMainTask(void *parameters);

QueueHandle_t getBleDeviceSentEventsQueue();
QueueHandle_t getBleNotifyToDeviceQueue();

#define READ_EVENT 0
#define WRITE_EVENT 1
#define NOTIFY_EVENT 2
#define STATUS_EVENT 3

struct BleDeviceSentEvent {
    uint8_t eventID;
    char eventValue[18];
};

struct BleNotifyDevice {
    bool isIndicate;
    char value[18];
};