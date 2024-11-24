#pragma once

#include <stdint.h>
#include <Arduino.h>

void TC4_start_timer();

void TC4_stop_timer();

void TC4_reset();

bool TC4_syncing();

void TC4_configure(uint32_t sample_period_seconds, uint8_t priority);

void TC4_reconfigure(uint32_t sample_period_seconds);

void TC3_start_timer();

void TC3_stop_timer();

void TC3_reset();

bool TC3_syncing();

void TC3_configure(uint8_t priority);