#pragma once

#include <stdint.h>

const inline char WEEKDAY_NAMES[7][4] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};
const inline char DEVICE_NAME[] = "Thermostat";
const inline char SERVER_NAME[] = "crumpster";

// Buttons
const uint8_t BUTTON_A_PIN = 9;
const uint8_t BUTTON_B_PIN = 10;

// Rotary Encoder
const uint8_t ROTARY_A_PIN = 2;
const uint8_t ROTARY_B_PIN = 4;
const uint8_t ROTARY_BTN_PIN = 3;

// Relay
const uint8_t STATUS_LED = 13;
const uint8_t FURNACE_RELAY_PIN = 6;
const uint8_t AC_RELAY_PIN = 5;

// Motion Sensor
const uint8_t MOTION_SENSOR_PIN = 11;

// I2C
const uint32_t I2C_FREQ = 400000;
