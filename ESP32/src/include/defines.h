
#define MAXESP32MESSAGELENGTH 50  //This is the length of the message recieved from the BBC Microbit
#define MAXMESSAGEFRAGMENTSIZE 40 

#define MAXBBCMESSAGELENGTH 40     //This is the length of the message sent to the BBC Microbit

//RTOS task priorities
#define MQTT_task_Priority	2
#define MQTT_client_task_Priority	4

#define switch_task_Priority 3
#define touch_task_Priority	3

#define routing_task_Priority 2

#define ServoEasingTask_Priority 5

#define ADC_task_Priority	1
#define DAC_task_Priority	1	
#define encoders_task_Priority	1
#define light_task_Priority	1
#define sound_task_Priority	1
#define sound_busy_task_Priority 1
