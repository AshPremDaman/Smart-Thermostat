#pragma once

#include <Wire.h>
#include <Arduino.h>
#include <RTCZero.h>
#include <WiFiNINA.h>

#include "Constants.hpp"
#include "Timers.hpp"
#include "MesgQueue.hpp"
#include "Messenger.hpp"

void initI2C();

void initGPIO();

void initTimers();

void initWiFi(Messenger& messenger);

void initRTC(RTCZero& rtc);