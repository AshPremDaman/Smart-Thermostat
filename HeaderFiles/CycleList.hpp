#pragma once

#include <stdint.h>

#include "Messenger.hpp"
#include "LinkedList.hpp"
#include "Messenger.hpp"
#include "parson.h"

typedef struct cycle
{
	int id;
	int start_hour;
	int end_hour;
	int start_min;
	int end_min;
	float temp_F;
} cycle_t;

class CycleList : public LinkedList<cycle_t>
{
private:
public:
	CycleList();
	~CycleList();
	void print_cycles();
	cycle_t *find_next_cycle(uint8_t hr, uint8_t min);
	void update_cycles(uint8_t day, Messenger& messenger);

	int m_dayID;
};
