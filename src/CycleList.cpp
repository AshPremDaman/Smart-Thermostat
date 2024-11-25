#include "CycleList.hpp"

CycleList::CycleList() : LinkedList()
{
	m_dayID = -1;
}

CycleList::~CycleList()
{
}

void CycleList::print_cycles()
{
}

cycle_t *CycleList::find_next_cycle(uint8_t hr, uint8_t min)
{
	for (auto current : *this)
	{
		if (current->start_hour > hr)
		{
			continue;
		}
		else if (current->start_hour == hr && current->start_min > min)
		{
			continue;
		}
		else
			return current;
	}
	return nullptr;
}

void CycleList::update_cycles(uint8_t day, Messenger& messenger)
{
	char buffer[1024];
	
	sprintf(buffer, URL_GET_CYCLES, day);

	int result = messenger.get_request(buffer, buffer, 1024);
	if (result < 1) {
		// request failed
		return;
	}
	// if successful, clear the cycle list
	this->clear();

	JSON_Value* raw = json_parse_string(buffer);
	JSON_Object* obj = json_value_get_object(raw);
	JSON_Array* cycles_array = json_object_get_array(obj, "cycles");

	int count = json_array_get_count(cycles_array);
	Serial.println(count);
	for (int i = 0; i < count; i++) {
		cycle c;
		JSON_Object* cycle_obj = json_array_get_object(cycles_array, i);
		
		c.id = (int)json_object_get_number(cycle_obj, "id");
		c.start_hour = (int)json_object_get_number(cycle_obj, "h");
		c.start_min = (int)json_object_get_number(cycle_obj, "m");
		c.temp_F = (float)json_object_get_number(cycle_obj, "t");
		this->push_front(c);
	}
	json_value_free(raw);

	int last_start_hour = 0;
	int last_start_min = 0;
	for (auto current : *this)
	{
		current->end_hour = last_start_hour;
		current->end_min = last_start_min;

		last_start_hour = current->start_hour;
		last_start_min = current->start_min;
	}
}