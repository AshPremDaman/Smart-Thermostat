#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>
#include "Messenger.hpp"

typedef struct {
	const uint8_t* icon;
	int8_t high;
	int8_t low;
	int8_t day;
} weather_data_t;

class Weather
{
	public:
		Weather();
		~Weather();
		bool is_new_day(int day);
		void get_weather(Messenger& messenger, int today);
		void set_current_weather(int hr);
		weather_data_t* get_current_weather();
		weather_data_t* get_forecast(int day);
	private:
		static const int FORECAST_HOURS = 25;
		static const int FORECAST_DAYS = 4;
		int current_day;
		const uint8_t* get_icon(int icon);
		weather_data_t* current_weather;
		weather_data_t* hourly_data[FORECAST_HOURS];
		weather_data_t* forecast_data[FORECAST_DAYS];
};
