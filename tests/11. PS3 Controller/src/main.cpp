#include <Ps3Controller.h>
#include <Stream.h>
#include <sstream>
#include <WiFi.h>

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

void onConnect()
{
  Serial.println("Connected!.");
}


void WiFiEvent(WiFiEvent_t event)
{
    //Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event)
    {
    case SYSTEM_EVENT_WIFI_READY:
        Serial.println("WiFi interface ready");
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        Serial.println("Completed scan for access points");
        break;
    case SYSTEM_EVENT_STA_START:
        Serial.println("WiFi client started");
        break;
    case SYSTEM_EVENT_STA_STOP:
        Serial.println("WiFi clients stopped");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        Serial.println("Connected to access point");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("Disconnected from WiFi access point");
        Serial.println("Reconnecting...");
        //WiFi.begin(storedSSID.c_str(), storedWifiPassword.c_str());
        //delay(1000);
        WiFi.reconnect();
        break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        Serial.println("Authentication mode of access point has changed");
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.print("Obtained IP address: ");
        Serial.print(WiFi.localIP());

        Serial.print(" in ");
        Serial.print(millis());
        Serial.println("ms");

        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        Serial.println("Lost IP address and IP address is reset to 0");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
        break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
        Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
        break;
    case SYSTEM_EVENT_AP_START:
        Serial.println("WiFi access point started");
        break;
    case SYSTEM_EVENT_AP_STOP:
        Serial.println("WiFi access point  stopped");
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        Serial.println("Client connected");
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        Serial.println("Client disconnected");
        break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
        Serial.println("Assigned IP address to client");
        break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
        Serial.println("Received probe request");
        break;
    case SYSTEM_EVENT_GOT_IP6:
        Serial.println("IPv6 is preferred");
        break;
    case SYSTEM_EVENT_ETH_START:
        Serial.println("Ethernet started");
        break;
    case SYSTEM_EVENT_ETH_STOP:
        Serial.println("Ethernet stopped");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        Serial.println("Ethernet connected");
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        Serial.println("Ethernet disconnected");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        Serial.println("Obtained IP address");
        break;
    default:
        break;
    }
}


void setup()
{
  Serial.begin(115200);
  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  Ps3.begin(); //01:02:03:04:05:06");

    //register wifi events
    WiFi.onEvent(WiFiEvent);

    WiFi.begin("the robot network","isaacasimov");
}

void loop()
{
  delay(1000);
}