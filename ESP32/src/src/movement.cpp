#include <Arduino.h>
#include "movement.h"
#include <iostream>
#include <string>
#include <vector>

extern SemaphoreHandle_t i2cSemaphore;

Adafruit_PWMServoDriver PCA9685 = Adafruit_PWMServoDriver();

//QueueHandle_t Movement_Queue;
QueueHandle_t Movement_i2c_Queue;

TaskHandle_t MovementTask;
TaskHandle_t Movementi2cTask;

std::vector<servo> servos;
//std::vector<TaskHandle_t> servoTasks;

void movement_setup()
{
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    //see if it's actually connected
    Wire.beginTransmission(64);
    auto error = Wire.endTransmission();

    if (error != 0)
    {
        Serial.println("SX1509 for switching not found");

        POST(5);
    }

    //don't need to call pwm.begin as this reinitialise the i2c
    //PCA9685.begin
    PCA9685.reset();
    PCA9685.setOscillatorFrequency(27000000);
    PCA9685.setPWMFreq(50); // Analog servos run at ~50 Hz updates

    checkI2Cerrors("movement");

    xSemaphoreGive(i2cSemaphore);

    delay(10);

    //set up struct array for the servos
    for (int16_t i = 0; i <= 15; i++)
    {
        servo S;

        S.pin = i;
        //S.PWM = 0;
        //S._change = 32;
        S.duration = 2;
        S.toDegree = 0;
        S.fromDegree = 0;
        //S.setDegree = 180;
        S.minPulse = 100;
        S.maxPulse = 505;
        //S.isMoving = false;
        S.easingCurve = LinearInOut;
        S.interuptEasing = false;
        S.taskHandle = NULL; //create the task handles

        servos.push_back(S);

        //create the task handles
        //servoTasks.push_back(NULL);
    }

    //Movement_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));

    //create a queue to hold servo PWM values (allows us to kill the servo easing processes at anytime)
    Movement_i2c_Queue = xQueueCreate(100, sizeof(servoPWM));

    //this task is to recieve the servo messages
    xTaskCreatePinnedToCore(
        movement_task,         /* Task function. */
        "Movement Task",       /* name of task. */
        3000,                  /* Stack size of task (2780) */
        NULL,                  /* parameter of the task */
        MovementTask_Priority, /* priority of the task */
        &MovementTask,         /* Task handle to keep track of created task */
        1);                    /* core */

    //this task is to perform pwms for the servos
    xTaskCreatePinnedToCore(
        movement_i2c_task,        /* Task function. */
        "Movement i2c Task",      /* name of task. */
        3000,                     /* Stack size of task (2700) */
        NULL,                     /* parameter of the task */
        Movementi2cTask_Priority, /* priority of the task */
        &Movementi2cTask,         /* Task handle to keep track of created task */
        1);                       /* core */
}

void movement_task(void *pvParameters)
{
    messageParts parts;

    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("movement_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    for (;;)
    {
        // Serial.println("waiting for Command_Queue");

        char msg[MAXESP32MESSAGELENGTH] = {0};

        //wait for new movement command in the queue
        xQueueReceive(Movement_Queue, &msg, portMAX_DELAY);

        // Serial.print("Movement_Queue:");
        // Serial.println(msg);

        //TODO: see if need this copy of msg
        std::string X = msg;

        parts = processQueueMessage(X.c_str(), "MOVEMENT");

        // Serial.print("action:");
        // Serial.print(parts.identifier);
        // Serial.print(" @ ");
        // Serial.println(millis());

        //OK OK - What ever your doing stop the servo task first
        auto stopPin = atol(parts.value1);
        stopServo(stopPin);

        if (strncmp(parts.identifier, "V1",2) == 0)
        {
            //not much to do now as we always stop the servos

            //Stop servo
            //auto pin = atol(parts.value1);
            //stopServo(pin);
        }
        else if (strncmp(parts.identifier, "V2",2) == 0)
        {
            //Set servo to angle
            auto pin = atoi(parts.value1);
            auto angle = atoi(parts.value2);
            auto minPulse = atoi(parts.value3);
            auto maxPulse = atoi(parts.value4);

            setServoAngle(pin, angle, minPulse, maxPulse);
        }
        else if (strncmp(parts.identifier, "V3",2) == 0)
        {
            //Set servo to angle
            auto pin = atoi(parts.value1);
            auto toDegree = atoi(parts.value2);
            auto fromDegree = atoi(parts.value3);
            auto duration = atoi(parts.value4);
            auto minPulse = atoi(parts.value5);
            auto maxPulse = atoi(parts.value6);

            setServoEase(pin, LinearInOut, toDegree, fromDegree, duration, minPulse, maxPulse);
        }
        else if (strncmp(parts.identifier, "V4",2) == 0)
        {
            //Set servo to angle
            auto pin = atoi(parts.value1);
            auto toDegree = atoi(parts.value2);
            auto fromDegree = atoi(parts.value3);
            auto duration = atoi(parts.value4);
            auto minPulse = atoi(parts.value5);
            auto maxPulse = atoi(parts.value6);

            setServoEase(pin, QuadraticInOut, toDegree, fromDegree, duration, minPulse, maxPulse);
        }
        else if (strncmp(parts.identifier, "V5",2) == 0)
        {
            //Serial.print("BounceInOut on pin ");

            //Set servo to angle
            auto pin = atoi(parts.value1);
            auto toDegree = atoi(parts.value2);
            auto fromDegree = atoi(parts.value3);
            auto duration = atoi(parts.value4);
            auto minPulse = atoi(parts.value5);
            auto maxPulse = atoi(parts.value6);

            setServoEase(pin, BounceInOut, toDegree, fromDegree, duration, minPulse, maxPulse);
        }

        else if (strncmp(parts.identifier, "V6",2) == 0)
        {
            //Set servo to PWM
            auto pin = atol(parts.value1);
            auto PWM = constrain(atol(parts.value2), 0, 4096);

            setServoPWM(pin, PWM);
        }
    }

    vTaskDelete(NULL);
}

void movement_i2c_task(void *pvParameters)
{
    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("movement_i2c_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    for (;;)
    {
        servoPWM pwm;

        //wait for the next PWM command to be queued - timing is one each loop making it simpler
        xQueueReceive(Movement_i2c_Queue, &pwm, portMAX_DELAY);

        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        // configure the PWM duty cycle
        PCA9685.setPWM(pwm.pin, 0, pwm.pwm);

        checkI2Cerrors("movement (movement_i2c_task)");

        xSemaphoreGive(i2cSemaphore);

        delay(1);
    }

    vTaskDelete(NULL);
}

void setServoEase(const int16_t pin, easingCurves easingCurve, const int16_t toDegree, const int16_t fromDegree, const int16_t duration, const int16_t minPulse, const int16_t maxPulse)
{
    //set variables
    //servos[pin].isMoving = true;
    //servos[pin]._change = 32;
    servos[pin].duration = duration;

    servos[pin].toDegree = toDegree;
    servos[pin].fromDegree = fromDegree;

    servos[pin].minPulse = minPulse;
    servos[pin].maxPulse = maxPulse;
    servos[pin].easingCurve = easingCurve;

    //servos[pin].PWM = 0;
    //servos[pin].setDegree = 0;

    servos[pin].interuptEasing = false;

    // Serial.printf(">>> change %f \t fromDegree %i \t toDegree %i \n", servos[pin]._change, servos[pin].fromDegree, servos[pin].toDegree);
    // Serial.println("setServoEase");

    const char *taskName = "Servo Task " + pin;

    xTaskCreatePinnedToCore(
        &ServoEasingTask,
        taskName, //"Servo Task",
        10000,
        NULL,
        ServoEasingTask_Priority,
        &servos[pin].taskHandle,
        1);

    // xTaskCreatePinnedToCore(
    //     &ServoEasingTask,
    //     taskName, //"Servo Task",
    //     10000,
    //     NULL,
    //     ServoEasingTask_Priority,
    //     &servoTasks[pin],
    //     1);

    xTaskNotify(servos[pin].taskHandle, pin, eSetValueWithOverwrite);

    //xTaskNotify(servoTasks[pin], pin, eSetValueWithOverwrite);

    delay(10);
}

void setServoPWM(const int16_t pin, const int16_t PWM)
{
    // Serial.println("setServoPWM");

    // Serial.print("setServoPWM on pin ");
    // Serial.print(pin);
    // Serial.print(" PWM  ");
    // Serial.print(PWM);
    // Serial.println("");

    servoPWM toBeQueued;
    toBeQueued.pin = pin;
    toBeQueued.pwm = PWM;

    xQueueSend(Movement_i2c_Queue, &toBeQueued, portMAX_DELAY);
}

void setServoAngle(const int16_t pin, const int16_t angle, const int16_t minPulse, const int16_t maxPulse)
{
    //Serial.println("setServoAngle");

    servos[pin].interuptEasing = false;
    //servos[pin].isMoving = false;
    servos[pin].minPulse = minPulse;
    servos[pin].maxPulse = maxPulse;
    //servos[pin].setDegree = angle;
    //servos[pin].PWM = mapAngles(servos[pin].setDegree, 0, 180, servos[pin].minPulse, servos[pin].maxPulse);

    int16_t PWM = mapAngles(angle, 0, 180, servos[pin].minPulse, servos[pin].maxPulse);

    //add to the queue
    setServoPWM(pin, PWM);
}

void stopServo(const int16_t pin)
{
    //Serial.printf("STOPPING SERVO PIN: %i\n", pin);
    //u_long startTime = millis();

    servos[pin].interuptEasing = true;

    //not super efficent - but easier, just loop and check
    //As much as this looks poor - it works and the loop can execute between 1-4 times before the task handle is NULL (0-400ms)
    while (true)
    {
        if (servos[pin].taskHandle == NULL)
        {
            break;
        }

        //just adding some delay to give the task enough time to react - the task already has a 50ms delay in it
        delay(50);

        // Serial.print("#");
    }

    servos[pin].interuptEasing = false;

    //give the task delete command some time to complete
    delay(10);

    // Serial.print("pin: ");
    // Serial.print(pin);
    // Serial.print(" servo stop completed in: ");
    // Serial.print(millis() - startTime);
    // Serial.print(" @ ");
    // Serial.println(millis());

    //Serial.printf("\nSTOPPING SERVO PIN: %i COMPLETED\n", pin);
}

void ServoEasingTask(void *pvParameter)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("ServoEasingTask uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    uint32_t pin = 0;
    BaseType_t xResult = xTaskNotifyWait(0X00, 0x00, &pin, portMAX_DELAY);

    // Serial.print("Servo easing task for pin: ");
    // Serial.print(pin);
    // Serial.print(" on core ");
    // Serial.println(xPortGetCoreID());

    u_long startTime = millis();

    auto duration = servos[pin].duration;

    auto toDegree = servos[pin].toDegree;
    auto fromDegree = servos[pin].fromDegree;

    auto minPulse = servos[pin].minPulse;
    auto maxPulse = servos[pin].maxPulse;
    auto easingCurve = servos[pin].easingCurve;

    auto fromDegreeMapped = mapAngles(fromDegree, 0, 180, minPulse, maxPulse);
    auto toDegreeMapped = mapAngles(toDegree, 0, 180, minPulse, maxPulse);

    auto _change = abs(fromDegreeMapped - toDegreeMapped);

    double easedPosition = 0;
    double t = 0;
    uint16_t PWM;
    uint16_t previousPWM;

    //Serial.printf("_change %f \t fromDegreeMapped %f \t toDegreeMapped %f \t fromDegree %i \t toDegree %i \n", _change, fromDegreeMapped, toDegreeMapped, fromDegree, toDegree);
    //Serial.printf("minPulse %i \t maxPulse %i \n", minPulse, maxPulse);

    //send message to microbit to indicate it's moving
    char msgtosend[MAXBBCMESSAGELENGTH];
    sprintf(msgtosend, "F3,%i", pin);
    sendToMicrobit(msgtosend);

    for (int i = 0; i <= duration * 20; i++)
    {
        switch (easingCurve)
        {
        case QuadraticInOut:
            easedPosition = QuarticEaseInOut(_change, duration, t);
            break;
        case BounceInOut:
            easedPosition = BounceEaseInOut(_change, duration, t);
            break;
        case LinearInOut:
            easedPosition = LinearEaseInOut(_change, duration, t);
            break;
        default:
            easedPosition = LinearEaseInOut(_change, duration, t);
        }

        t += 0.05; //this works with delay(50)

        //work out the PWM based on the direction of travel
        if (fromDegreeMapped > toDegreeMapped)
        {
            //PWM = easedPosition + minPulse;
            PWM = fromDegreeMapped - easedPosition;
        }
        else
        {
            // PWM = maxPulse - easedPosition;
            PWM = fromDegreeMapped + easedPosition;
        }

        //Serial.printf("i: %d \t easedPosition: %f \t PWM: %i \t t: %f \t %f \t %f \n", i, easedPosition, servos[0].PWM, t, servos[0]._change, servos[0]._duration);

        //only send if a differance to reduce i2c traffic
        if (PWM != previousPWM)
        {
            //add to the queue
            setServoPWM(pin, PWM);
        }

        //remember the previous PWM
        previousPWM = PWM;

        //take a very defined break
        delay(50);

        //check for the message to interupt early
        if (servos[pin].interuptEasing == true)
        {
            //jump out of the for loop
            break;
        }
    }

    // Serial.print("pin: ");
    // Serial.print(pin);
    // Serial.print(" loop completed in: ");
    // Serial.print(millis() - startTime);
    // Serial.print(" @ ");
    // Serial.println(millis());

    //servos[pin].isMoving = false;

    if (servos[pin].interuptEasing == false)
    {
        //Add event to BBC microbit queue
        char msgtosend[MAXBBCMESSAGELENGTH];
        sprintf(msgtosend, "F1,%i,%lu", pin, millis() - startTime);
        sendToMicrobit(msgtosend);
    }
    else
    {
        //send message to microbit - Servo 0-15 has stopped due STOP command during easing
        char msgtosend[MAXBBCMESSAGELENGTH];
        sprintf(msgtosend, "F2,%i,%lu", pin, millis() - startTime);
        sendToMicrobit(msgtosend);
    }

    /* The task is going to be deleted. Set the handle to NULL. */
    servos[pin].taskHandle = NULL;

    //delete task
    vTaskDelete(NULL);
}

double mapAngles(const double x, const double in_min, const double in_max, const double out_min, const double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
