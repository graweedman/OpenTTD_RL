#include "communication.h"

RLCommunication::RLCommunication() : connected(false) {
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	sock = INVALID_SOCKET;
#else
	sock = -1;
#endif
};

RLCommunication::~RLCommunication() {
	Close();
#ifdef _WIN32
	WSACleanup();
#endif
}

bool RLCommunication::Connect(const std::string &host, int port) {
#ifdef _WIN32
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) return false;
#else
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) return false;
#endif
	addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(host.c_str());
	memcpy(&this->sock, &addr, sizeof(addr));

	connected = true;
    return true;
}

bool RLCommunication::Send(const std::vector<float> &data) {
	if (!connected) return false;

	int bytes = sendto(
		sock,
		reinterpret_cast<const char *>(data.data()),
		static_cast<int>(data.size() * sizeof(float)),
		0,
		(sockaddr *)&addr,
		sizeof(addr)
	);

	return bytes == static_cast<int>(data.size() * sizeof(float));
}

bool RLCommunication::Receive(std::vector<float> &data, int max_size) {
	if (!connected) return false;
	data.resize(max_size / sizeof(float));
	sockaddr_in from;
#ifdef _WIN32
	int fromlen = sizeof(from);
#else
	socklen_in fromlen = sizeof(from);
#endif
	int bytes = recvfrom(
		sock,
		reinterpret_cast<char*>(data.data()),
		max_size,
		0,
		(sockaddr*)&from,
		&fromlen
	);
	if (bytes < 0) return false;
	data.resize(bytes / sizeof(float));
	return true;
}

void RLCommunication::Close() {
	if (!connected) return;
#ifdef _WIN32
	closesocket(sock);
	sock = INVALID_SOCKET;
#else
	close(sock);
	sock = -1;
#endif
	connected = false;
}






