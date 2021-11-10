#pragma once

//RTOS task priorities (highest is 24)
#define MQTT_task_Priority 10
#define MQTT_client_task_Priority 10

#define switch_task_Priority 10 
#define touch_task_Priority 10

#define MovementTask_Priority 12
#define Movementi2cTask_Priority 14
#define ServoEasingTask_Priority 10

#define LEDBlinkingTask_Priority 10
#define LEDBreathingTask_Priority 10

#define DAC_task_Priority 10
#define encoders_task_Priority 10 
#define light_task_Priority 10
#define sound_task_Priority 10
#define sound_busy_task_Priority 10

#define BBC_RX_Priority 15
#define BBC_TX_Priority 15

//UART variables
#define BBC_UART_NUM UART_NUM_2

#define UARTMESSAGELENGTH 128
#define MAXBBCMESSAGELENGTH 128
 
#define PATTERN_LEN 1
#define PATTERN_FROM_MICROBIT 13 //0x7f

#define RX_BUF_SIZE 1024
#define TX_BUF_SIZE 1024  

/*
#define MQTT_task_Priority 12
#define MQTT_client_task_Priority 12

#define switch_task_Priority 5 
#define touch_task_Priority 7

#define MovementTask_Priority 14
#define Movementi2cTask_Priority 15
#define ServoEasingTask_Priority 14

#define LEDBlinkingTask_Priority 10
#define LEDBreathingTask_Priority 10

#define DAC_task_Priority 5
#define encoders_task_Priority 5 
#define light_task_Priority 5
#define sound_task_Priority 5
#define sound_busy_task_Priority 5

#define BBC_RX_Priority 20
#define BBC_TX_Priority 19
*/