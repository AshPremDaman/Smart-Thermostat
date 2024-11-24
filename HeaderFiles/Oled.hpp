#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <time.h>

#include "Constants.hpp"
#include "Thermostat.hpp"
#include "History.hpp"
#include "Weather.hpp"
#include "Messenger.hpp"
#include "Logging.hpp"

#define OLED_NUM_MENUS 7

#define OLED_RESET -1		// Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_WIDTH 128	// OLED display width, in pixels
#define SCREEN_HEIGHT 64	// OLED display height, in pixels
#define BUFFER_SIZE SCREEN_HEIGHT *SCREEN_WIDTH / 8

#define OLED_TITLE_X 0
#define OLED_TITLE_Y 0

#define OLED_LINE_1_X 0
#define OLED_LINE_1_Y 17

#define OLED_LINE_2_X 0
#define OLED_LINE_2_Y 26

#define OLED_LINE_3_X 0
#define OLED_LINE_3_Y 35

#define OLED_LINE_4_X 0
#define OLED_LINE_4_Y 44

#define FONT_SIZE_TITLE 2
#define FONT_SIZE_LINE 1

class OLED
{
public:
	OLED(Adafruit_SSD1306* display, tm* clk, History* history, Messenger* messenger, Weather* weather, Logging* logger, thermostat_settings* settings, sensor_readings* sensor);
	~OLED();
	void next_menu();
	void previous_menu();
	void edit();
	void update();
	void on();
	void off();
	void rotary_dial(uint8_t direction);
	void set_runtime(uint32_t runtime);
	int8_t get_filter();

private:
	void menu_weather_forecast();
	void draw_history_graph();
	void menu_sensor_data();
	void menu_sample_settings();
	void menu_set_temperature();
	void menu_set_thresholds();
	void menu_motion_settings();
	void menu_date_and_time();
	void menu_current_cycle();
	void menu_wifi_status();

	void update_user_settings();
	void update_temporary_setting_value();
	void bound_scroll_counter(int8_t low, int8_t high, uint8_t height);

	Adafruit_SSD1306 *m_display;
	thermostat_settings *m_settings;
	sensor_readings *m_sensor;
	History* m_history;
	Messenger* m_messenger;
	Weather* m_weather;
	Logging* m_logger;
	tm *m_time;
	uint32_t m_furnace_runtime;
	int8_t oled_scroll_counter;
	int8_t oled_menu_state;
	int8_t oled_menu_item;
	int8_t temporary_setting;
	int8_t m_filter;
	char buffer[25];
	bool edit_oled_menu;
	bool oled_screen_ON;
	bool update_temporary;
	bool save_temporary;
	static int8_t line_selection[];
	enum menu_items
	{
		TARGET,
		THRESHOLDLOWER,
		THRESHOLDUPPER,
		BASELINE,
		TOTALSAMPLES,
		SAMPLEPERIOD,
		FILTERTYPE,
		SCREENTIMEOUT,
		MOTIONDETECTION,
		SYNCRTC,
		TOGGLE_WIFI,
		OVERRIDE
	};
};
