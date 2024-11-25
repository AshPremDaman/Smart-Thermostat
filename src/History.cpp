#include "History.hpp"

#include <arduino.h>

History::History()
{
	m_data_points = new float[m_length];
	m_read_index = 0;
	m_write_index = 0;
}

History::~History()
{
}

void History::initialize(float t) {
	m_curr_max = t;
	m_curr_min = t;

	for (int i = 0; i < m_length; i++) {
		m_data_points[i] = t;
	}
}

void History::insert(float t) {
	m_data_points[m_write_index++] = t;

	if (m_write_index >= m_length) {
		m_write_index = 0;
	}
}

int16_t mapper(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int16_t History::get_datapoint() {
	float t = m_data_points[m_read_index++];
	if (m_read_index >= m_length) {
		m_read_index = 0;
	}

	return mapper(t, m_curr_min, m_curr_max, m_grid_y_min, m_grid_y_max);
}

void History::prepare_data() {
	m_curr_max = 30.0;
	m_curr_min = 120.0;

	for(int i = 0; i < m_length; i++) {
		m_curr_max = max(m_curr_max, m_data_points[i]);
		m_curr_min = min(m_curr_min, m_data_points[i]);
	}
	m_curr_min = floor(m_curr_min);
	m_curr_max = ceil(m_curr_max);

	m_read_index = m_write_index;
}

float History::get_max() {
	return m_curr_max;
}

float History::get_min() {
	return m_curr_min;
}

void History::set_grid_scale(float y_min, float y_max) {
	m_grid_y_max = y_max;
	m_grid_y_min = y_min;
}
