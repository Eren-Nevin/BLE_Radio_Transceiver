#ifndef Arduino_h
#include <Arduino.h>
#endif

void start_dispatch();

// Serial Comm:

void writeToSerial(char* message);
void writeToSerial(const char *message_param);
void writeToSerial(std::string message);

// Bluetooth Comm:

void notifyBleDevices(bool isIndicate, char * payload);
// returns the callback index in the callbacks array.
int8_t addCallbackForBleDevicesEvents(void callback(uint8_t eventID, char value[18]));
bool removeCallbackForBleDevicesEvents(uint8_t callback_index);

// Radio Comm:

void sendCodeToRadio(uint32_t value);
void putRadioIntoLearning();
void putRadioIntoIdle();
void cancelRadioLearning();
// Change the value to cover both protocol and the actual code itself.
int8_t addCallbackForRadioStatus(void callback(uint8_t status, uint32_t value));
bool removeCallbackForRadioDevicesEvents(uint8_t callback_index);