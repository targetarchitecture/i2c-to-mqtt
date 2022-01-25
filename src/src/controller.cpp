#include <Arduino.h>
#include "controller.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_spp_api.h"

void ps3_spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
#define PS3_TAG "PS3_SPP"
#define PS3_DEVICE_NAME "PS3 Host"
#define PS3_SERVER_NAME "PS3_SERVER" 

    ESP_LOGE(PS3_TAG, "%s begin event %i\n", __func__, event);

    if (event == ESP_SPP_INIT_EVT) {
        ESP_LOGE(PS3_TAG, "ESP_SPP_INIT_EVT");
        esp_bt_dev_set_device_name(PS3_DEVICE_NAME);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
        ESP_LOGE(PS3_TAG, "esp_bt_gap_set_scan_mode");
#else
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE);
        ESP_LOGE(PS3_TAG, "esp_bt_gap_set_scan_mode");
#endif

        esp_spp_start_srv(ESP_SPP_SEC_NONE,ESP_SPP_ROLE_SLAVE, 0, PS3_SERVER_NAME);
    }
}

void controller_setup()
{

    Ps3.attach(notify);
    Ps3.attachOnConnect(onConnect);
    Ps3.begin();
    //String address = Ps3.getAddress();

    // Serial.print("The ESP32's Bluetooth MAC address is: ");
    // Serial.println(address);

    // Ps3.begin("98:f4:ab:67:8e:8e");
    //Ps3.begin(address.c_str());

    // delay(100);
}

void onConnect()
{
    Serial.println("PS3 Connected!.");
}

void notify()
{
    if (Ps3.data.button.cross)
    {
        Serial.println("Pressing the cross button");
    }

    if (Ps3.data.button.square)
    {
        Serial.println("Pressing the square button");
    }

    if (Ps3.data.button.triangle)
    {
        Serial.println("Pressing the triangle button");
    }

    if (Ps3.data.button.circle)
    {
        Serial.println("Pressing the circle button");
    }
}
