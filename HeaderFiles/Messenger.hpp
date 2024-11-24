#pragma once

#include <stdint.h>
#include <WiFiNINA.h>
#include <WIFIUdp.h>

#include "parson.h"
#include "Constants.hpp"
#include "WiFiSecrets.h"
#include "MesgQueue.hpp"

enum SERVER{
	TEMPORARY = 1,
	SCHEDULE_UPDATED = 2,
	NODE_RED = 3,
	FLASK = 4,
	SERVER_UP = 5
};

class Messenger
{
private:
	uint8_t m_wifi_connection_status;

	// Local listening port
	const uint16_t LOCAL_PORT = 2390;
	const uint16_t LOCAL_HB_PORT = 2391;

	// Server ports
	const uint16_t SERVER_GET_PORT = 5000;
	const uint16_t SERVER_POST_PORT = 1880;
	const uint16_t SERVER_HB_PORT = 1887;

	// IP address of server
	IPAddress m_remoteIP;
	String m_server_str;
	bool m_server_found;
	bool m_wifi_enabled;

	// Local connection
	IPAddress m_localIP;
	const char* m_ssid;

	// UDP sockets
	WiFiUDP m_udp_hb;
	WiFiUDP m_udp_msg;

	// Methods
	bool check_server_available(const SERVER msg);

public:
	Messenger(/* args */);
	~Messenger();

	// Methods
	void initialize();
	String& get_server_str();
	bool obtain_server_IP();
	bool server_found();
	bool wifi_connected();
	const char* get_ssid();
	IPAddress& get_localIP();
	uint8_t connect_to_wifi(int tries);
	uint8_t auto_reconnect_wifi();
	void disconnect_wifi();
	bool post_request(const char *path, const char *msg, int len);
	bool post_request(const char *path, const char *msg, int len, uint16_t port);
	int get_request(const char *path, char *buffer, size_t size);
	int check_inbox();
	float get_temporary_temperature();
	bool get_day_ids(int *id_array);
	uint32_t get_epoch();
};

const inline char* URL_GET_TEMPORARY = "/getTemporary";
const inline char* URL_GET_FORECAST = "/getForecast";
const inline char* URL_GET_DAY_IDS = "/getDayIDs";
const inline char* URL_GET_CYCLES = "/getCycles/%d";
const inline char* URL_GET_EPOCH = "/getEpoch";

const inline char* URL_TEMPERATURE = "/temp";
const inline char* URL_STATS = "/stats";
const inline char* URL_MOTION = "/motion";
const inline char* URL_RUNTIME = "/runtime";
const inline char* URL_FURNANCE_STATE = "/furnaceState";

const inline char* POST_TEMPERATURE = "TEMP=%f&HUM=%f";
const inline char* POST_STATS = "TARGET=%f&THRESH_L=%f&THRESH_H=%f";
const inline char* POST_MOTION = "MOTION=%d";
const inline char* POST_RUNTIME = "RUNTIME=%d";
const inline char* POST_FURNACE_STATE = "F_State=%d";