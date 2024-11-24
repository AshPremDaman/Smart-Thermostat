
#ifndef THERMOSTAT_HPP
#define THERMOSTAT_HPP

#include <Arduino_FreeRTOS.h>

// Function declarations for FreeRTOS tasks
void TaskBlinkExternal(void *pvParameters);
void TaskDisplayNumber(void *pvParameters);
void TaskReadSensor(void *pvParameters);
void TaskTurnOnFan(void *pvParameters);
void TaskSetFanTemp(void *pvParameters);

#endif
