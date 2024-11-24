#pragma once

#include <stdint.h>

class History
{
private:
	/* data */
	int m_read_index;
	int m_write_index;
	float m_curr_max;
	float m_curr_min;
	float* m_data_points;
	float m_grid_y_max;
	float m_grid_y_min;

public:
	History();
	~History();
	void initialize(float t);
	void insert(float t);
	void set_grid_scale(float y_min, float y_max);
	void prepare_data();
	int16_t get_datapoint();
	float get_max();
	float get_min();

	const int m_length = 110;
};

