#pragma once

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string>
#include <vector>
#include <iostream>

class RLCommunication {
public:
	RLCommunication();
	~RLCommunication();

	bool Connect(const std::string& host, int port);
	bool Send(const std::vector<float>& data);
	bool Receive(std::vector<float>& data, int max_size = 4096);
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