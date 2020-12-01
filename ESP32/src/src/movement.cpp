#include <Arduino.h>
#include "movement.h"

volatile servo servos[15] = {0};
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
extern SemaphoreHandle_t i2cSemaphore;
TaskHandle_t MovementTask;

void movement_setup()
{
    //Serial.println("8 channel Servo test!");

    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    //Serial.println("xSemaphoreTake");

    //see if it's actually connected
    Wire.beginTransmission(64);
    auto error = Wire.endTransmission();

    if (error != 0)
    {
        Serial.println("SX1509 for switching not found");

        POST(5);
    }

    //don't need to call pwm.begin as this reinitialise the i2c
    //pwm.begin
    pwm.reset();
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(50); // Analog servos run at ~50 Hz updates

    checkI2Cerrors("movement");

    xSemaphoreGive(i2cSemaphore);

    delay(10);

    //set up struct array for the servos
    for (int16_t i = 0; i < 15; i++)
    {
        servos[i].pin = i;
        servos[i].PWM = 0;
        servos[i]._change = 32;
        servos[i]._duration = 2;
        servos[i].toDegree = 0;
        servos[i].fromDegree = 0;
        servos[i].setDegree = 180;
        servos[i].minPulse = 100;
        servos[i].maxPulse = 505;
        servos[i].isMoving = false;
        servos[i].easingCurve = LinearInOut;
        servos[i].interuptEasing = false;
    }

    //this task is to recieve the
    xTaskCreate(
        movement_task,                /* Task function. */
        "Movement Task",              /* name of task. */
        configMINIMAL_STACK_SIZE * 4, /* Stack size of task */
        NULL,                         /* parameter of the task */
        2,                            /* priority of the task */
        &MovementTask);               /* Task handle to keep track of created task */
}

void movement_task(void *pvParameters)
{
    messageParts parts;
    //UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("movement_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    Serial.printf("Movement task is on core %i\n", xPortGetCoreID());

    for (;;)
    {
        // Serial.println("waiting for Command_Queue");

        char msg[MAXMESSAGELENGTH] = {0};

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

        if (strcmp(parts.identifier, "V1") == 0)
        {
            //Stop servo
            auto pin = atol(parts.value1);

            stopServo(pin);
        }
        else if (strcmp(parts.identifier, "V2") == 0)
        {
            //Set servo to angle
            auto pin = atol(parts.value1);
            auto angle = constrain(atol(parts.value2), 0, 180);
            auto minPulse = constrain(atol(parts.value3), 0, 4096);
            auto maxPulse = constrain(atol(parts.value4), 0, 4096);

            setServoAngle(pin, angle, minPulse, maxPulse);
        }
        else if (strcmp(parts.identifier, "V3") == 0)
        {
            //Set servo to angle
            auto pin = atol(parts.value1);
            auto easing = parts.value2;
            auto toDegree = constrain(atol(parts.value3), 0, 180);
            auto fromDegree = constrain(atol(parts.value4), 0, 180);
            auto duration = atol(parts.value5);
            auto minPulse = constrain(atol(parts.value6), 0, 4096);
            auto maxPulse = constrain(atol(parts.value7), 0, 4096);
            easingCurves easingCurve;

            if (strcmp(parts.identifier, "LINEAR") == 0)
            {
                easingCurve = LinearInOut;
            }
            else if (strcmp(parts.identifier, "BOUNCE") == 0)
            {
                easingCurve = BounceInOut;
            }
            else if (strcmp(parts.identifier, "QUADRATIC") == 0)
            {
                easingCurve = BounceInOut;
            }
            else
            {
                easingCurve = LinearInOut;
            }

            setServoEase(pin, easingCurve, toDegree, fromDegree, duration, minPulse, maxPulse);
        }
        else if (strcmp(parts.identifier, "V4") == 0)
        {
            //Set servo to PWM
            auto pin = atol(parts.value1);
            auto PWM = constrain(atol(parts.value2), 0, 4096);

            setServoPWM(pin, PWM);
        }
    }

    vTaskDelete(NULL);
}

void setServoEase(const int16_t pin, easingCurves easingCurve, const int16_t toDegree, const int16_t fromDegree, const int16_t duration, const int16_t minPulse, const int16_t maxPulse)
{
    //set variables
    servos[pin].isMoving = true;
    servos[pin]._change = 32;
    servos[pin]._duration = duration;

    servos[pin].toDegree = toDegree;
    servos[pin].fromDegree = fromDegree;

    servos[pin].minPulse = minPulse;
    servos[pin].maxPulse = maxPulse;
    servos[pin].easingCurve = easingCurve;

    servos[pin].PWM = 0;
    servos[pin].setDegree = 0;

    servos[pin].interuptEasing = false;

    //Serial.println("setServoEase");

    //uxTaskGetStackHighWaterMark = 9750
    if (pin == 0)
    {
        xTaskCreate(&ServoEasingTask, "Servo 0 Task", 10000, (void *)0, 3, NULL);
    }
    if (pin == 1)
    {
        xTaskCreate(&ServoEasingTask, "Servo 1 Task", 10000, (void *)1, 3, NULL);
    }
    if (pin == 2)
    {
        xTaskCreate(&ServoEasingTask, "Servo 2 Task", 10000, (void *)2, 3, NULL);
    }
    if (pin == 3)
    {
        xTaskCreate(&ServoEasingTask, "Servo 3 Task", 10000, (void *)3, 3, NULL);
    }
    if (pin == 4)
    {
        xTaskCreate(&ServoEasingTask, "Servo 4 Task", 10000, (void *)4, 3, NULL);
    }
    if (pin == 5)
    {
        xTaskCreate(&ServoEasingTask, "Servo 5 Task", 10000, (void *)5, 3, NULL);
    }
    if (pin == 6)
    {
        xTaskCreate(&ServoEasingTask, "Servo 6 Task", 10000, (void *)6, 3, NULL);
    }
    if (pin == 7)
    {
        xTaskCreate(&ServoEasingTask, "Servo 7 Task", 10000, (void *)7, 3, NULL);
    }
    if (pin == 8)
    {
        xTaskCreate(&ServoEasingTask, "Servo 8 Task", 10000, (void *)8, 3, NULL);
    }
    if (pin == 9)
    {
        xTaskCreate(&ServoEasingTask, "Servo 9 Task", 10000, (void *)9, 3, NULL);
    }
    if (pin == 10)
    {
        xTaskCreate(&ServoEasingTask, "Servo 10 Task", 10000, (void *)10, 3, NULL);
    }
    if (pin == 11)
    {
        xTaskCreate(&ServoEasingTask, "Servo 11 Task", 10000, (void *)11, 3, NULL);
    }
    if (pin == 12)
    {
        xTaskCreate(&ServoEasingTask, "Servo 12 Task", 10000, (void *)12, 3, NULL);
    }
    if (pin == 13)
    {
        xTaskCreate(&ServoEasingTask, "Servo 13 Task", 10000, (void *)13, 3, NULL);
    }
    if (pin == 14)
    {
        xTaskCreate(&ServoEasingTask, "Servo 14 Task", 10000, (void *)14, 3, NULL);
    }
    if (pin == 15)
    {
        xTaskCreate(&ServoEasingTask, "Servo 15 Task", 10000, (void *)15, 3, NULL);
    }
}

void setServoPWM(const int16_t pin, const int16_t PWM)
{
    // Serial.println("setServoPWM");

    servos[pin].PWM = PWM;

    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    pwm.setPWM(pin, 0, PWM);

    checkI2Cerrors("movement (setServoPWM)");

    xSemaphoreGive(i2cSemaphore);

    // Serial.print("setServoPWM on pin ");
    // Serial.print(pin);
    // Serial.print(" PWM  ");
    // Serial.print(PWM);
    // Serial.println("");
}

void setServoAngle(const int16_t pin, const int16_t angle, const int16_t minPulse, const int16_t maxPulse)
{
    //Serial.println("setServoAngle");

    // call stop servo to stop the servo
    stopServo(pin);

    //servos[pin].interuptEasing = false;
    //servos[pin].isMoving = false;
    servos[pin].minPulse = minPulse;
    servos[pin].maxPulse = maxPulse;
    servos[pin].setDegree = angle;
    servos[pin].PWM = mapAngles(servos[pin].setDegree, 0, 180, servos[pin].minPulse, servos[pin].maxPulse);

    //wait for the i2c semaphore flag to become available

    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    pwm.setPWM(pin, 0, servos[pin].PWM);

    checkI2Cerrors("movement (setServoAngle)");

    xSemaphoreGive(i2cSemaphore);
}

void stopServo(const int16_t pin)
{
    //Serial.println("stopServo");

    // set the stopEasing value to the stop the task
    servos[pin].interuptEasing = true;

    //just changed this to a small delay as the code below takes time as it run sequentially.
    delay(10);
}

void ServoEasingTask(void *pvParameter)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("Servo1TaskNew uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    int pin = (uintptr_t)pvParameter;

    Serial.print("Servo easing task for pin: ");
    Serial.print(pin);
    Serial.print(" on core ");
    Serial.println(xPortGetCoreID());

    u_long startTime = millis();

    auto fromDegreeMapped = mapAngles(servos[pin].fromDegree, 0, 180, servos[pin].minPulse, servos[pin].maxPulse);
    auto toDegreeMapped = mapAngles(servos[pin].toDegree, 0, 180, servos[pin].minPulse, servos[pin].maxPulse);

    servos[pin]._change = abs(fromDegreeMapped - toDegreeMapped);

    double easedPosition = 0;
    double t = 0;

    //Serial.printf("_change %f \t fromDegreeMapped %f \t toDegreeMapped %f \t fromDegree %i \t toDegree %i \n", servos[0]._change, fromDegreeMapped, toDegreeMapped, servos[0].fromDegree, servos[0].toDegree);

    for (int i = 0; i <= servos[pin]._duration * 20; i++)
    {
        switch (servos[pin].easingCurve)
        {
        case QuadraticInOut:
            easedPosition = QuarticEaseInOut(servos[pin]._change, servos[pin]._duration, t);
            break;
        case BounceInOut:
            easedPosition = BounceEaseInOut(servos[pin]._change, servos[pin]._duration, t);
            break;
        case LinearInOut:
            easedPosition = LinearEaseInOut(servos[pin]._change, servos[pin]._duration, t);
            break;
        default:
            easedPosition = LinearEaseInOut(servos[pin]._change, servos[pin]._duration, t);
        }

        t += 0.05; //this works with delay(50)

        //work out the PWM based on the direction of travel
        if (fromDegreeMapped < toDegreeMapped)
        {
            servos[pin].PWM = easedPosition + servos[pin].minPulse;
        }
        else
        {
            servos[pin].PWM = servos[pin].maxPulse - easedPosition;
        }

        //printf("i: %d \t easedPosition: %f \t PWM: %i \t t: %f \t %f \t %f \n", i, easedPosition, servos[0].PWM, t, servos[0]._change, servos[0]._duration);

        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        pwm.setPWM(pin, 0, servos[pin].PWM);

        checkI2Cerrors("movement (ServoEasingTask)");

        xSemaphoreGive(i2cSemaphore);

        //check for marker to stop easing
        if (servos[pin].interuptEasing == true)
        {
            // Serial.print("Request to leave easing task: ");
            // Serial.println(millis());

            break;
        }

        delay(50);
    }

    //Add event to BBC microbit queue
    char msgtosend[MAXBBCMESSAGELENGTH];

    if (servos[pin].interuptEasing == false)
    {
        sprintf(msgtosend, "F1,%d", pin);
    }
    else
    {
        sprintf(msgtosend, "F2,%d", pin);
    }
    sendToMicrobit(msgtosend);

    // Serial.print("completed in: ");
    // Serial.print(millis() - startTime);
    // Serial.print(" @ ");
    // Serial.println(millis());

    servos[pin].isMoving = false;
    servos[pin].interuptEasing = false;

    //delete task
    vTaskDelete(NULL);
}

double mapAngles(const double x, const double in_min, const double in_max, const double out_min, const double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
