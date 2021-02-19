#include <Arduino.h>
#include "BLE.h"

// TaskHandle_t BLETask;


// void BLE_task(void *pvParameters)
// {
//     UBaseType_t uxHighWaterMark;
//     uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
//     Serial.print("BLE_task uxTaskGetStackHighWaterMark:");
//     Serial.println(uxHighWaterMark);

//     for (;;)
//     {
//         if (deviceConnected)
//         {
// Serial.println("BLE_task 1");

//             pTxCharacteristic->setValue(&txValue, 1);
//             pTxCharacteristic->notify();
//             txValue++;
//             delay(10); // bluetooth stack will go into congestion, if too many packets are sent
//         }

//         // disconnecting
//         if (!deviceConnected && oldDeviceConnected)
//         {
//             delay(500);                  // give the bluetooth stack the chance to get things ready
//             pServer->startAdvertising(); // restart advertising
//             Serial.println("start advertising");
//             oldDeviceConnected = deviceConnected;
//         }
//         // connecting
//         if (deviceConnected && !oldDeviceConnected)
//         {
//             // do stuff here on connecting
//             oldDeviceConnected = deviceConnected;
//         }
//     }

//     vTaskDelete(NULL);
// }