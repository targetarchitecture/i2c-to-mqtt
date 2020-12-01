#include <Arduino.h>
#include <MPR121.h>
#include <Wire.h>
#include "SN4 pins.h"
#include <WiFi.h>
#include "Constants.h"

MPR121 mpr121;

void Wifi_connect();

void setup()
{
  Wifi_connect();

  //start i2c
  Wire.begin(SDA, SCL);

  Serial.begin(constants::baud);

  mpr121.setupSingleDevice(*constants::wire_ptr,
                           constants::device_address,
                           constants::fast_mode);

  mpr121.setAllChannelsThresholds(constants::touch_threshold,
                                  constants::release_threshold);
  mpr121.setDebounce(constants::device_address,
                     constants::touch_debounce,
                     constants::release_debounce);
  mpr121.setBaselineTracking(constants::device_address,
                             constants::baseline_tracking);
  mpr121.setChargeDischargeCurrent(constants::device_address,
                                   constants::charge_discharge_current);
  mpr121.setChargeDischargeTime(constants::device_address,
                                constants::charge_discharge_time);
  mpr121.setFirstFilterIterations(constants::device_address,
                                  constants::first_filter_iterations);
  mpr121.setSecondFilterIterations(constants::device_address,
                                   constants::second_filter_iterations);
  mpr121.setSamplePeriod(constants::device_address,
                         constants::sample_period);

  mpr121.startChannels(constants::physical_channel_count,
                       constants::proximity_mode);
}

void loop()
{
  delay(constants::loop_delay);

  if (!mpr121.communicating(constants::device_address))
  {
    Serial.printf("mpr121 device not commmunicating!\n\n");
    return;
  }

  uint16_t touch_status = mpr121.getTouchStatus(constants::device_address);
  bool any_touched = mpr121.anyTouched(touch_status);

  if (any_touched == true)
  {
    Serial.printf("any_touched:  %i \n", any_touched);

    for (uint8_t i = 0; i < 12; i++)
    {
      // it if *is* touched and *wasnt* touched before, alert!
      if ((touch_status & _BV(i)))
      {
         Serial.print(i);
         Serial.println(" touched");
      }
    }

    uint8_t channel_count = mpr121.getChannelCount();
    Serial.printf("channel_count: %i \n", channel_count);

    uint8_t running_channel_count = mpr121.getRunningChannelCount();
    Serial.printf("running_channel_count: %i \n", running_channel_count);

    Serial.printf("touch_status:  %i \n", touch_status);

    if (mpr121.overCurrentDetected(touch_status))
    {
      Serial.printf("Over current detected!\n\n");
      mpr121.startChannels(constants::physical_channel_count,
                           constants::proximity_mode);
      return;
    }

    bool device_channel_touched = mpr121.deviceChannelTouched(touch_status,
                                                              constants::channel);
    Serial.printf("device_channel_touched:  %i \n", device_channel_touched);

    uint16_t out_of_range_status = mpr121.getOutOfRangeStatus(constants::device_address);
    Serial.printf("out_of_range_status: %i \n", out_of_range_status);

    bool channel_touched = mpr121.channelTouched(constants::channel);
    Serial.printf("channel_touched:  %i \n", channel_touched);

    uint16_t channel_filtered_data = mpr121.getChannelFilteredData(constants::channel);
    Serial.printf("channel_filtered_data: %i \n", channel_filtered_data);

    uint16_t channel_baseline_data = mpr121.getChannelBaselineData(constants::channel);
    Serial.printf("channel_baseline_data: %i \n", channel_baseline_data);
  }
}

void Wifi_connect()
{
  Serial.println("Connecting to Wifi");
  Serial.println("152 2.4GHz");
  Serial.println("derwenthorpe");

  WiFi.mode(WIFI_STA);

  //connect
  while (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin("152 2.4GHz", "derwenthorpe");
    delay(500);
    Serial.println(WiFi.status());
  }
}
