#pragma once
#include <Arduino.h>
#include <queue>
#include "Messenger.hpp"

class Logging
{
	public:
	Logging(Messenger* messenger);
	bool send();
	void debug(const String& msg);
	void info(const String& msg);
	void warn(const String& msg);
	void error(const String& msg);
	private:
	void log(const String& level, const String& msg);
	void encode(String& msg);
	Messenger* m_messenger;
	std::queue<String> m_queue;
	const int MAX_SIZE = 25;
	const String DEBUG = "DEBUG";
	const String INFO = "INFO";
	const String WARN = "WARN";
	const String ERROR = "ERROR";
};
