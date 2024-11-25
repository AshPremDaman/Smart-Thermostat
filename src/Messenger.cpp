#include "Messenger.hpp"

Messenger::Messenger(/* args */)
{
	m_server_found = false;
	m_wifi_enabled = true;
	m_wifi_connection_status = WL_DISCONNECTED;
	m_server_str = "Unknown";
	m_ssid = "Unknown";
}

Messenger::~Messenger()
{
}

void Messenger::initialize() {
	
	m_udp_msg.begin(LOCAL_PORT);
	m_udp_hb.begin(LOCAL_HB_PORT);
}

uint8_t Messenger::connect_to_wifi(int tries)
{
	m_wifi_enabled = true;
	m_wifi_connection_status = WiFi.status();
	while (m_wifi_connection_status != WL_CONNECTED && tries-- > 0) {
		m_wifi_connection_status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
		delay(500);
	};
	
	if (m_wifi_connection_status == WL_CONNECTED)
	{
		m_ssid = WiFi.SSID();
		m_localIP = WiFi.localIP();
		global_msg_queue->push(CONNECT_TO_SERVER);
	}
	return m_wifi_connection_status;
}

uint8_t Messenger::auto_reconnect_wifi()
{
	if (!m_wifi_enabled)
		return m_wifi_connection_status;
	m_wifi_connection_status = WiFi.status();
	if (m_wifi_connection_status != WL_DISCONNECTED && m_wifi_connection_status != WL_CONNECTION_LOST)
		return m_wifi_connection_status;
	return connect_to_wifi(1);
}

void Messenger::disconnect_wifi()
{
	m_wifi_enabled = false;
	m_wifi_connection_status = WiFi.disconnect();
	m_server_str = "Unknown";
	m_server_found = false;
	m_ssid = "Unknown";
	m_remoteIP = IPAddress();
	m_localIP = IPAddress();
}

IPAddress& Messenger::get_localIP()
{
	return m_localIP;
}

const char* Messenger::get_ssid()
{
	return m_ssid;
}

String& Messenger::get_server_str()
{
	return m_server_str;
}

bool Messenger::obtain_server_IP()
{
	int result = WiFi.hostByName(SERVER_NAME, m_remoteIP);
	m_server_found = result == 1;
	if (m_server_found)
	{
		m_server_str = String(m_remoteIP[0]);
		m_server_str += "." + String(m_remoteIP[1]);
		m_server_str += "." + String(m_remoteIP[2]);
		m_server_str += "." + String(m_remoteIP[3]);

		Serial.println("FOUND SERVER");
	}
	return m_server_found;
}

bool Messenger::wifi_connected()
{
	return m_wifi_connection_status == WL_CONNECTED;
}

bool Messenger::server_found()
{
	return m_server_found;
}

bool Messenger::check_server_available(const SERVER msg) {

	m_udp_hb.beginPacket(m_remoteIP, SERVER_HB_PORT);
	m_udp_hb.write(String(msg).c_str());
	m_udp_hb.endPacket();

	uint8_t buffer[8];
	int size;
	uint32_t start = millis();
	do {
		size = m_udp_hb.parsePacket();
	}
	while(size == 0 && millis() - start < 300);
	
	if (size > 0) {
		m_udp_hb.read(buffer, 8);
		return true;
	}
	else {
		return false;
	} 
}

bool Messenger::post_request(const char *path, const char *msg, int len) {
	return post_request(path, msg, len, SERVER_POST_PORT);
}

bool Messenger::post_request(const char *path, const char *msg, int len, uint16_t port) {
	if (!m_server_found)
	{
		return false;
	}
	if (!check_server_available(NODE_RED))
	{
		Serial.println("Server unavailable");
		return false;
	}

	// Prepare the client
	WiFiClient client;

	// Connect to the server
	if (client.connect(m_remoteIP, port))
	{
		// Connection established
		client.print("POST ");
		client.print(path);
		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.print(m_server_str);
		client.print(":");
		client.println(port);
		client.println("Content-Type: application/x-www-form-urlencoded");
		client.print("Content-Length: ");
		client.println(len);
		client.println();
		client.println(msg);
		client.println();
		client.stop();
		return true;
	}
	client.stop();
	return false;
}

int Messenger::get_request(const char *path, char *buffer, size_t size) {
	if (!m_server_found)
	{
		return -1;
	}
	if (!check_server_available(FLASK))
	{
		Serial.println("Server unavailable");
		return -1;
	}

	// Prepare the client
	WiFiClient client;
	int data_length = -1;

	// Connect to the server
	if (client.connect(m_remoteIP, SERVER_GET_PORT))
	{
		// Connection established
		client.print("GET ");
		client.print(path);
		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.print(m_server_str);
		client.print(":");
		client.println(SERVER_GET_PORT);
		client.println();
	}
	else
	{
		// Failed dot connect
		Serial.println("FAILED TO CONNECT");
		client.stop();
		return -1;
	}

	// Listen and manage a timeout
	unsigned long startTime = millis();
	bool received = false;

	while ((millis() - startTime < 2000) && !received)
	{

		while (client.available())
		{
			received = true;
			// check for valid response
			if (!client.find("HTTP/1.1"))
			{
				Serial.println("NO HTTP 1.1");
				client.stop();
				return -1;
			}
			int status_code = client.parseInt();
			if (!client.find("Content-Length:"))
			{
				Serial.println("Cannot find content length");
				client.stop();
				return -1;
			}
			int content_lenght = client.parseInt();
			// if HTTP status OK seek to the data portion of the response
			if (status_code == 200 && client.find("\n\r\n"))
			{
				Serial.println("READING DATA");
				data_length = client.readBytes(buffer, content_lenght);
				// set null terminator
				buffer[data_length] = '\0';
			}
		}
	}
	client.stop();

	return data_length;
}

int Messenger::check_inbox() {
	int packet_size = m_udp_msg.parsePacket();
	if (packet_size == 1) {
		// read packet into buffer
		char buffer[16];
		int len = m_udp_msg.read(buffer, 16);
		buffer[len] = 0;
		return atoi(buffer);
	}
	return packet_size;
}

float Messenger::get_temporary_temperature() {
	char buffer[256];
	float temperature = 0.0;
	if (get_request(URL_GET_TEMPORARY, buffer, 256) > 0)
	{
		JSON_Value *raw = json_parse_string(buffer);
		JSON_Object *obj = json_value_get_object(raw);
		temperature = (float)json_object_get_number(obj, "temporary");
		json_value_free(raw);
	}
	return temperature;
}

bool Messenger::get_day_ids(int *id_array) {
	char buffer[256];
	if (get_request(URL_GET_DAY_IDS, buffer, 256) > 0)
	{
		JSON_Value *raw = json_parse_string(buffer);
		JSON_Object *obj = json_value_get_object(raw);
		
		for (int i = 0; i < 7; i++)
		{
			id_array[i] = (int)json_object_get_number(obj, WEEKDAY_NAMES[i]);
		}
		json_value_free(raw);
		return true;
	}
	return false;
}

uint32_t Messenger::get_epoch() {
	char buffer[256];
	uint32_t epoch = 0;
	if (get_request(URL_GET_EPOCH, buffer, 256) > 0)
	{
		JSON_Value *raw = json_parse_string(buffer);
		JSON_Object *obj = json_value_get_object(raw);
		epoch = (uint32_t)json_object_get_number(obj, "epoch");
		json_value_free(raw);
	}
	return epoch;
}
