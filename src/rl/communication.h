#pragma once

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#endif

#include <iostream>
#include <string>
#include <vector>
// #include <iostream>
// #include "../stdafx.h"

class RLCommunication {
public:
	RLCommunication();
	~RLCommunication();

	bool Connect(const std::string &host, int remote_port, int local_port = 0);
	bool Send(const std::vector<uint8_t>& data);
	bool Send(const std::vector<float>& data);
	bool Receive(std::vector<uint8_t>& data, int max_size = 32);
	bool IsConnected() const { return connected; }

	void Close();
private:
#ifdef _WIN32
	SOCKET sock;
#else
	int sock;
#endif
	bool connected;
	sockaddr_in addr; // Address structure for the socket
};