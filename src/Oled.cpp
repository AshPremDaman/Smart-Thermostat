#include "Oled.hpp"

OLED::OLED(Adafruit_SSD1306 *display, tm *clk, History* history, Messenger* messenger, Weather* weather, Logging* logger, thermostat_settings *settings, sensor_readings *sensor)
{
	m_settings = settings;
	m_sensor = sensor;
	m_time = clk;
	m_history = history;
	m_messenger = messenger;
	m_weather = weather;
	m_logger = logger;
	m_history->set_grid_scale(55.0, 18.0);
	m_display = display;
	m_display->setTextColor(SSD1306_WHITE);
	edit_oled_menu = false;
	oled_menu_item = 0;
	oled_menu_state = 0;
	oled_screen_ON = true;
	oled_scroll_counter = 0;
	m_furnace_runtime = 0;
	m_filter = Thermostat::Temperature_Filters::AVERAGE;
	String s;
}

OLED::~OLED()
{
}

int8_t OLED::get_filter()
{
	return m_filter;
}

void OLED::update()
{

	if (!oled_screen_ON)
	{
		return;
	}

	switch (oled_menu_state)
	{
	case 0:
	{
		menu_weather_forecast();
		break;
	}
	case 1:
	{
		menu_set_temperature();
		break;
	}
	case 2:
	{
		draw_history_graph();
		break;
	}
	case 3:
	{
		menu_wifi_status();
		break;
	}
	case 4:
	{
		menu_sensor_data();
		break;
	}
	case 5:
	{
		menu_set_thresholds();
		break;
	}
	case 6:
	{
		menu_sample_settings();
		break;
	}
	case 7:
	{
		menu_motion_settings();
		break;
	}
	case 8:
	{
		menu_date_and_time();
		break;
	}
	case 9:
	{
		menu_current_cycle();
		break;
	}
	default:
	{
		if (oled_menu_state > 9)
		{
			oled_menu_state = 0;
		}
		else if (oled_menu_state < 0) {
			oled_menu_state = 9;
		}
		update();
		break;
	}
	}
}

void OLED::next_menu()
{
	if (oled_screen_ON)
	{
		oled_menu_state++;
		oled_scroll_counter = 0;
		edit_oled_menu = false;
		oled_menu_item = -1;
	}

	oled_screen_ON = true;
	update();
}

void OLED::previous_menu()
{
	if (oled_screen_ON)
	{
		oled_menu_state--;
		oled_scroll_counter = 0;
		edit_oled_menu = false;
		oled_menu_item = -1;
	}

	oled_screen_ON = true;
	update();
}

void OLED::edit()
{
	if (oled_screen_ON)
	{
		edit_oled_menu = !edit_oled_menu;

		if (edit_oled_menu)
		{ // If editing, get the corrisponding variable and copy it to a temporary variable
			update_temporary_setting_value();
		}
		else
		{ // If not editing set the temporary to the corrisponding var
			update_user_settings();
		}
	}

	oled_screen_ON = true;
	update();
}

void OLED::on()
{
	oled_screen_ON = true;
	update();
}

void OLED::off()
{
	oled_screen_ON = false;
	m_display->clearDisplay();
	m_display->display();
}

void OLED::rotary_dial(uint8_t direction)
{
	if (oled_screen_ON)
	{
		if (edit_oled_menu)
		{
			temporary_setting += direction;
		}
		else
		{
			oled_scroll_counter += direction;
		}
	}
	else
	{
		oled_screen_ON = true;
	}
	update();
}

void OLED::set_runtime(uint32_t runtime)
{
	m_furnace_runtime = runtime;
}

void OLED::menu_set_temperature()
{
	int8_t items[] = {-1, TARGET};

	// Clear OLED buffer
	m_display->clearDisplay();
	m_display->setTextSize(FONT_SIZE_TITLE);
	m_display->setCursor(0, 0);

	// Draw the title
	m_display->println("Edit Temp");

	// Convert temperature to string
	sprintf(buffer, "Air:%.1fF\367", m_sensor->temperature_F);
	// Draw room temperature label at line 1
	m_display->setCursor(1, OLED_LINE_1_Y);
	m_display->println(buffer);

	// Convert target temperature to string
	if (edit_oled_menu && oled_menu_item == TARGET)
	{
		sprintf(buffer, "Set:%d.0F\367", temporary_setting);
	}
	else
	{
		sprintf(buffer, "Set:%.1fF\367", m_settings->target_temperature);
	}

	// Draw target temperature at line 3
	m_display->setCursor(1, OLED_LINE_3_Y);
	m_display->println(buffer);

	// Confine the scroll line of the menu
	bound_scroll_counter(3, 3, 16);

	// Save which item we are editing
	oled_menu_item = items[(oled_scroll_counter - 2)];

	// Send the buffer to OLED RAM
	m_display->display();
}

void OLED::menu_sensor_data()
{
	// Clear OLED buffer
	m_display->clearDisplay();
	m_display->setTextSize(FONT_SIZE_TITLE);
	m_display->setCursor(0, 0);

	// Draw the title
	m_display->println("Sensors");
	m_display->setTextSize(FONT_SIZE_LINE);

	// Convert celcius temperature to string
	sprintf(buffer, "HDC1080:  %.1f C\367", m_sensor->temperature_C);

	// Draw celcius temperature at line 1
	m_display->setCursor(1, OLED_LINE_1_Y);
	m_display->println(buffer);

	// Convert fahrenheit temperature to string
	sprintf(buffer, "HDC1080:  %.1f F\367", m_sensor->temperature_F);

	// Draw fahrenheit temperature at line 2
	m_display->setCursor(1, OLED_LINE_2_Y);
	m_display->println(buffer);

	// Convert humidity to string
	sprintf(buffer, "Humidity: %.1f %%", m_sensor->humidity);

	// Draw humidity at line 3
	m_display->setCursor(1, OLED_LINE_3_Y);
	m_display->println(buffer);

	// Convert average temperature to string
	sprintf(buffer, "Average:  %.1f F\367", m_sensor->average_temperature);

	// Draw humidity at line 4
	m_display->setCursor(1, OLED_LINE_4_Y);
	m_display->println(buffer);

	// Send the buffer to OLED RAM
	m_display->display();
}

void OLED::menu_sample_settings()
{
	int8_t items[] = {-1, TOTALSAMPLES, SAMPLEPERIOD, FILTERTYPE};

	// Clear OLED buffer
	m_display->clearDisplay();
	m_display->setTextSize(FONT_SIZE_TITLE);
	m_display->setCursor(0, 0);

	// Draw the title
	m_display->println("Settings");
	m_display->setTextSize(FONT_SIZE_LINE);

	// Switch between edited value and set value
	if (edit_oled_menu && oled_menu_item == TOTALSAMPLES)
	{
		sprintf(buffer, "Total Samples: %d", temporary_setting);
	}
	else
	{
		sprintf(buffer, "Total Samples: %d", m_settings->total_samples);
	}
	// Draw total samples at line 1
	m_display->setCursor(1, OLED_LINE_1_Y);
	m_display->println(buffer);

	// Switch between edited value and set value
	if (edit_oled_menu && oled_menu_item == SAMPLEPERIOD)
	{
		sprintf(buffer, "Sample Period: %ds", temporary_setting);
	}
	else
	{
		sprintf(buffer, "Sample Period: %ds", m_settings->sample_period_sec);
	}
	// Draw sample period in seconds at line 2
	m_display->setCursor(1, OLED_LINE_2_Y);
	m_display->println(buffer);
	int8_t filter = m_filter;
	if (edit_oled_menu && oled_menu_item == FILTERTYPE)
	{
		filter = temporary_setting % 3;
	}
	
	if (filter == Thermostat::Temperature_Filters::KALMAN){
		sprintf(buffer, "Filter: %s", "Kalman");
	}
	else if (filter == Thermostat::Temperature_Filters::AVERAGE){
		sprintf(buffer, "Filter: %s", "Average");
	}
	else if (filter == Thermostat::Temperature_Filters::NONE){
		sprintf(buffer, "Filter: %s", "None");
	}

	m_display->setCursor(1, OLED_LINE_3_Y);
	m_display->println(buffer);

	sprintf(buffer, "K-estimate:  %.1f F\367", m_sensor->k_estimate);

	// k-estimate at line 4
	m_display->setCursor(1, OLED_LINE_4_Y);
	m_display->println(buffer);

	// Confine the scroll line of the menu
	bound_scroll_counter(1, 3, 9);

	// Save which item we are editing
	oled_menu_item = items[oled_scroll_counter];

	// Send the buffer to OLED RAM
	m_display->display();
}

void OLED::menu_set_thresholds()
{
	int8_t items[] = {-1, THRESHOLDLOWER, THRESHOLDUPPER, BASELINE};

	// Clear OLED buffer
	m_display->clearDisplay();
	m_display->setTextSize(FONT_SIZE_TITLE);
	m_display->setCursor(0, 0);

	// Draw the title
	m_display->println("Settings");
	m_display->setTextSize(FONT_SIZE_LINE);
	// Switch between edited value and set value
	if (edit_oled_menu && oled_menu_item == THRESHOLDLOWER)
	{
		sprintf(buffer, "Thresh Lower:  %d.0 F\367", temporary_setting);
	}
	else
	{
		sprintf(buffer, "Thresh Lower:  %.1f F\367", m_settings->lower_threshold);
	}
	// Draw lower threshold at line 1
	m_display->setCursor(1, OLED_LINE_1_Y);
	m_display->println(buffer);

	// Switch between edited value and set value
	if (edit_oled_menu && oled_menu_item == THRESHOLDUPPER)
	{
		sprintf(buffer, "Thresh Upper:  %d.0 F\367", temporary_setting);
	}
	else
	{
		sprintf(buffer, "Thresh Upper:  %.1f F\367", m_settings->upper_threshold);
	}
	// Draw upper threshold at line 2
	m_display->setCursor(1, OLED_LINE_2_Y);
	m_display->println(buffer);

	// Switch between edited value and set value
	if (edit_oled_menu && oled_menu_item == BASELINE)
	{
		sprintf(buffer, "Baseline:     %d.0 F\367", temporary_setting);
	}
	else
	{
		sprintf(buffer, "Baseline:     %.1f F\367", m_settings->baseline_temperature);
	}

	// Draw baseline temperautre at line 3
	m_display->setCursor(1, OLED_LINE_3_Y);
	m_display->println(buffer);

	// Confine the scroll line of the menu
	bound_scroll_counter(1, 3, 9);

	// Save which item we are editing
	oled_menu_item = items[oled_scroll_counter];

	// Send the buffer to OLED RAM
	m_display->display();
}

void OLED::menu_motion_settings()
{
	int8_t items[] = {-1, SCREENTIMEOUT, OVERRIDE, MOTIONDETECTION};

	// Clear OLED buffer
	m_display->clearDisplay();
	m_display->setTextSize(FONT_SIZE_TITLE);
	m_display->setCursor(0, 0);

	// Draw the title
	m_display->println("Settings");
	m_display->setTextSize(FONT_SIZE_LINE);

	// Switch between edited value and set value
	if (edit_oled_menu && oled_menu_item == SCREENTIMEOUT)
	{
		sprintf(buffer, "Screen Timeout: %ds", temporary_setting);
	}
	else
	{
		sprintf(buffer, "Screen Timeout: %ds", (int)(m_settings->screen_timeout_millis / 1000));
	}
	// Draw a screen timeout at line 1
	m_display->setCursor(1, OLED_LINE_1_Y);
	m_display->println(buffer);

	// Switch between edited value and set value
	if (edit_oled_menu && oled_menu_item == OVERRIDE)
	{
		sprintf(buffer, "Override: %d min", temporary_setting);
	}
	else
	{
		// Also, covnert millis to minutes
		sprintf(buffer, "Override: %d min", (int)(m_settings->override_timeout_millis / (60 * 1000)));
	}
	// Draw away time at line 2
	m_display->setCursor(1, OLED_LINE_2_Y);
	m_display->println(buffer);

	// Switch between edited value and set value
	if (edit_oled_menu && oled_menu_item == MOTIONDETECTION)
	{
		sprintf(buffer, "Away Time: %d hr", temporary_setting);
	}
	else
	{
		// Also, covnert millis to hours
		sprintf(buffer, "Away Time: %d hr", (int)(m_settings->motion_timeout_millis / (3600 * 1000)));
	}
	// Draw away time at line 3
	m_display->setCursor(1, OLED_LINE_3_Y);
	m_display->println(buffer);

	// Confine the scroll line of the menu
	bound_scroll_counter(1, 3, 9);

	// Save which item we are editing
	oled_menu_item = items[oled_scroll_counter];

	// Send the buffer to OLED RAM
	m_display->display();
}

void OLED::menu_date_and_time()
{

	int8_t items[] = {-1, -1, SYNCRTC};

	// Clear OLED buffer
	m_display->clearDisplay();
	m_display->setTextSize(FONT_SIZE_TITLE);
	m_display->setCursor(0, 0);

	// Draw the title
	m_display->println("Time");
	m_display->setTextSize(FONT_SIZE_LINE);

	// Convert time into date string
	char *c = asctime(m_time);
	c[16] = ' ';

	c[17] = c[20];
	c[18] = c[21];
	c[19] = c[22];
	c[20] = c[23];

	c[21] = '\0';

	// Draw time/date on line 1
	m_display->setCursor(1, OLED_LINE_1_Y);
	m_display->println(c);

	// Draw sync option on line 2
	m_display->setCursor(1, OLED_LINE_2_Y);
	m_display->println("Sync RTC Time  'OK'");

	// Confine the scroll line of the menu
	bound_scroll_counter(2, 2, 9);

	// Save which item we are editing
	oled_menu_item = items[oled_scroll_counter];

	// Send the buffer to OLED RAM
	m_display->display();
}

void OLED::menu_current_cycle()
{
	// Clear OLED buffer
	m_display->clearDisplay();
	m_display->setTextSize(FONT_SIZE_TITLE);
	m_display->setCursor(0, 0);

	// Draw the title
	m_display->println("Cycle Info");
	m_display->setTextSize(FONT_SIZE_LINE);

	// Draw start time on line 1
	sprintf(buffer, "Start time %02d:%02d", m_settings->current_cycle->start_hour, m_settings->current_cycle->start_min);
	m_display->setCursor(1, OLED_LINE_1_Y);
	m_display->println(buffer);

	// Draw end time on line 2
	sprintf(buffer, "End time   %02d:%02d", m_settings->current_cycle->end_hour, m_settings->current_cycle->end_min);
	m_display->setCursor(1, OLED_LINE_2_Y);
	m_display->println(buffer);

	// Draw runtime on line 3
	sprintf(buffer, "Runtime %d min", (int)(m_furnace_runtime / 60));
	m_display->setCursor(1, OLED_LINE_3_Y);
	m_display->println(buffer);

	// Send the buffer to OLED RAM
	m_display->display();
}

void OLED::menu_wifi_status()
{
	int8_t items[] = {-1, -1, -1, -1, TOGGLE_WIFI};
	// Clear OLED buffer
	m_display->clearDisplay();
	m_display->setTextSize(FONT_SIZE_TITLE);
	m_display->setCursor(0, 0);

	// Draw the title
	m_display->println("WiFi");
	m_display->setTextSize(FONT_SIZE_LINE);

	// Draw SSID on line 1
	m_display->setCursor(1, OLED_LINE_1_Y);
	m_display->print("SSID: ");
	m_display->println(m_messenger->get_ssid());

	// Draw local ip on line 2
	m_display->setCursor(1, OLED_LINE_2_Y);
	m_display->print("LCL IP: ");
	m_display->println(m_messenger->get_localIP());

	// Draw server ip on line 3
	m_display->setCursor(1, OLED_LINE_3_Y);
	m_display->print("SRV IP: ");
	m_display->println(m_messenger->get_server_str());

	// Draw connect/disconnect option on line 4
	m_display->setCursor(1, OLED_LINE_4_Y);
	if (m_messenger->wifi_connected())
	{
		m_display->print("Disconnect");
	}
	else
	{
		m_display->print("Connect");
	}
	m_display->println(" WiFi  'OK'");

	bound_scroll_counter(4, 4, 9);

	// Save which item we are editing
	oled_menu_item = items[oled_scroll_counter];

	// Send the buffer to OLED RAM
	m_display->display();
}

void OLED::draw_history_graph()
{
	// Clear OLED buffer
	m_display->clearDisplay();
	m_display->setCursor(0, 0);

	// Draw the title
	m_display->setTextSize(FONT_SIZE_TITLE);
	m_display->print("History");
	m_display->setTextSize(FONT_SIZE_LINE);
	
	// Convert celcius temperature to string
	sprintf(buffer, " %.1fF\367", m_sensor->temperature_F);
	m_display->println(buffer);
	
	m_history->prepare_data();
	// Y-axis labels
	m_display->setCursor(0, 17);
	m_display->print((int16_t)m_history->get_max());
	m_display->setCursor(0, 48);
	m_display->print((int16_t)m_history->get_min());

	// X-axis labels
	int time_marker = (m_settings->sample_period_sec * m_history->m_length) / 60;
	m_display->setCursor(116, 57);
	m_display->print(0);
	m_display->setCursor(61, 57);
	m_display->print(time_marker >> 1);
	m_display->setCursor(13, 57);
	m_display->print(time_marker);
	m_display->drawFastHLine(12, 55, 112, WHITE);
	m_display->drawFastVLine(12, 17, 39, WHITE);

	for (int x = 13; x < m_history->m_length+13; x++) {
		int16_t y = m_history->get_datapoint();
		m_display->drawPixel(x, y, WHITE);
	}

	// Send the buffer to OLED RAM
	m_display->display();
}

void OLED::update_user_settings()
{
	switch (oled_menu_item)
	{
	case TARGET:
	{
		if (temporary_setting >= m_settings->baseline_temperature && temporary_setting <= 95)
		{
			m_settings->target_temperature = temporary_setting;
			global_msg_queue->push(START_TEMPORARY_OVERRIDE);
			global_msg_queue->push(SEND_SERVER_STATS);
		}
	}
	break;
	case THRESHOLDLOWER:
	{
		if (temporary_setting >= 0 && temporary_setting <= 5)
		{
			m_settings->lower_threshold = temporary_setting;
			global_msg_queue->push(SEND_SERVER_STATS);
			m_logger->info("Set lower threshold to: " + String(temporary_setting));
		}
	}
	break;
	case THRESHOLDUPPER:
	{
		if (temporary_setting >= 0 && temporary_setting <= 5)
		{
			m_settings->upper_threshold = temporary_setting;
			global_msg_queue->push(SEND_SERVER_STATS);
			m_logger->info("Set upper threshold to: " + String(temporary_setting));
		}
	}
	break;
	case BASELINE:
	{
		if (temporary_setting >= 45 && temporary_setting <= 60)
		{
			m_settings->baseline_temperature = temporary_setting;
			m_logger->info("Set baseline temperature to: " + String(temporary_setting));
		}
	}
	break;
	case TOTALSAMPLES:
	{
		if (temporary_setting >= 1 && temporary_setting <= 20)
		{
			m_settings->total_samples = temporary_setting;
			global_msg_queue->push(UPDATE_SAMPLE_SUM);
			m_logger->info("Updated number of samples used to: " + String(temporary_setting));
		}
	}
	break;
	case SAMPLEPERIOD:
	{
		if (temporary_setting >= 5 && temporary_setting <= 300)
		{
			m_settings->sample_period_sec = temporary_setting;
			global_msg_queue->push(UPDATE_SAMPLE_PERIOD);
			m_logger->info("Updated sample period (sec) to: " + String(temporary_setting));
		}
	}
	break;
	case FILTERTYPE:
	{
		m_filter = temporary_setting % 3;
		global_msg_queue->push(SET_FILTER);
	}
	break;
	case SCREENTIMEOUT:
	{
		if (temporary_setting >= 1 && temporary_setting <= 180)
		{
			m_settings->screen_timeout_millis = temporary_setting * 1000;
			m_logger->info("Set screen timout (sec) to: " + String(temporary_setting));
		}
	}
	break;
	case MOTIONDETECTION:
	{
		if (temporary_setting >= 6 && temporary_setting <= 96)
		{
			m_settings->motion_timeout_millis = temporary_setting * 3600 * 1000; // convert to milliseconds from hours
			m_logger->info("Set motion timeout (hrs) to: " + String(temporary_setting));
		}
	}
	case OVERRIDE:
	{
		if (temporary_setting >= 15 && temporary_setting <= 180)
		{
			m_settings->override_timeout_millis = temporary_setting * 60 * 1000; // convert to milliseconds from minutes
			m_logger->info("Set temporary timeout (min) to: " + String(temporary_setting));
		}
	}
	break;
	default:
		break;
	}
}

void OLED::update_temporary_setting_value()
{
	switch (oled_menu_item)
	{
	case TARGET:
	{
		temporary_setting = m_settings->target_temperature;
	}
	break;
	case THRESHOLDLOWER:
	{
		temporary_setting = m_settings->lower_threshold;
	}
	break;
	case THRESHOLDUPPER:
	{
		temporary_setting = m_settings->upper_threshold;
	}
	break;
	case BASELINE:
	{
		temporary_setting = m_settings->baseline_temperature;
	}
	break;
	case TOTALSAMPLES:
	{
		temporary_setting = m_settings->total_samples;
	}
	break;
	case SAMPLEPERIOD:
	{
		temporary_setting = m_settings->sample_period_sec;
	}
	break;
	case SCREENTIMEOUT:
	{
		temporary_setting = m_settings->screen_timeout_millis / 1000; // convert from millis to seconds
	}
	break;
	case MOTIONDETECTION:
	{
		temporary_setting = m_settings->motion_timeout_millis / (3600 * 1000); // convert from millis to hours
	}
	break;
	case OVERRIDE:
	{
		temporary_setting = m_settings->override_timeout_millis / (60 * 1000); // convert from millis to minutes
	}
	break;
	case TOGGLE_WIFI:
	{
		edit_oled_menu = false;
		if (m_messenger->wifi_connected())
		{
			global_msg_queue->push(DISCONNECT_WIFI);
		}
		else
		{
			global_msg_queue->push(CONNECT_TO_WIFI);
		}
		global_msg_queue->push(OLED_UPDATE);
	}
	break;
	case SYNCRTC:
	{
		edit_oled_menu = false;
		global_msg_queue->push(GET_EPOCH);
		global_msg_queue->push(RTC_UPDATE);
		global_msg_queue->push(OLED_UPDATE);
	}
	break;
	default:
		break;
	}
}

void OLED::bound_scroll_counter(int8_t low, int8_t high, uint8_t height)
{
	if (oled_scroll_counter > high)
	{
		oled_scroll_counter = high;
	}
	else if (oled_scroll_counter < low - 1)
	{
		oled_scroll_counter = low - 1;
	}
	if (oled_scroll_counter >= low)
		m_display->fillRect(0, line_selection[oled_scroll_counter] - 1, SCREEN_WIDTH, height, SSD1306_INVERSE);
}

int8_t OLED::line_selection[] = {-20, OLED_LINE_1_Y, OLED_LINE_2_Y, OLED_LINE_3_Y, OLED_LINE_4_Y};

void OLED::menu_weather_forecast()
{
	m_display->clearDisplay();

	// Draw the title
	m_display->setCursor(0, 0);
	m_display->setTextSize(FONT_SIZE_TITLE);
	m_display->print("Weather");
	m_display->setCursor(105, 0);
	m_display->setTextSize(FONT_SIZE_LINE);

	weather_data_t* wd = m_weather->get_current_weather();

	if (oled_scroll_counter > 3 || oled_scroll_counter < 0)
	{
		oled_scroll_counter = 0;
	}
	switch (oled_scroll_counter)
	{
	case 0:
		m_display->print("now");
		m_display->setTextSize(FONT_SIZE_TITLE);

		sprintf(buffer, "O:%dF\367", wd->high);
		m_display->setCursor(56, OLED_LINE_1_Y);
		m_display->println(buffer);

		sprintf(buffer, "I:%dF\367", (int)m_sensor->temperature_F);
		m_display->setCursor(56, OLED_LINE_3_Y);
		m_display->println(buffer);
		break;
	case 1:
	case 2:
	case 3:
		wd = m_weather->get_forecast(oled_scroll_counter);
		m_display->print(WEEKDAY_NAMES[wd->day]);
		m_display->setTextSize(FONT_SIZE_TITLE);

		sprintf(buffer, "H:%dF\367", wd->high);
		m_display->setCursor(56, OLED_LINE_1_Y);
		m_display->println(buffer);

		sprintf(buffer, "L:%dF\367", wd->low);
		m_display->setCursor(56, OLED_LINE_3_Y);
		m_display->println(buffer);
	break;
	default:
		break;
	}
	m_display->drawBitmap(0, 16, wd->icon, 48, 48, SSD1306_WHITE);

	m_display->display();
}
