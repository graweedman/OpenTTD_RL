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

bool RLCommunication::Connect(const std::string &host, int remote_port, int local_port) {
#ifdef _WIN32
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return false;

	u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return false;
    
    // Set non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return false;  // Check for error
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) return false;  // Check for error
#endif

    // Set up remote address
    addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote_port);
    if (inet_aton(host.c_str(), &addr.sin_addr) == 0) {  // Better than inet_addr
        return false;
    }

    // Bind to local port (0 = let OS choose, or specify different port)
    sockaddr_in local_addr = {};
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(local_port);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        close(sock);  // Clean up on failure
        sock = -1;
        return false;
    }

    connected = true;
    return true;
}

// bool RLCommunication::Send(const std::vector<uint8_t> &data) {
// 	if (!connected) return false;
// 	int bytes = sendto(
// 		sock,
// 		reinterpret_cast<const char *>(data.data()),
// 		static_cast<int>(data.size()),
// 		0,
// 		(sockaddr *)&addr,
// 		sizeof(addr)
// 	);

// 	return bytes == static_cast<int>(data.size());
// }

bool RLCommunication::Send(const std::vector<float> &data) {
    if (!connected) return false;
    int bytes = sendto(
        sock,
        reinterpret_cast<const char *>(data.data()),
        static_cast<int>(data.size() * sizeof(float)), // send the correct number of bytes
        0,
        (sockaddr *)&addr,
        sizeof(addr)
    );

    return bytes == static_cast<int>(data.size() * sizeof(float));
}

bool RLCommunication::Receive(std::vector<uint8_t> &data, int max_size) {
	if (!connected) return false;
	data.resize(max_size);
	sockaddr_in from;
#ifdef _WIN32
	int fromlen = sizeof(from);
#else
	socklen_t fromlen = sizeof(from);
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
	data.resize(bytes);
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






