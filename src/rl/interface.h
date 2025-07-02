#ifndef RL_INTERFACE_H
#define RL_INTERFACE_H

#include "../stdafx.h"
#include "../town.h"
#include "../industry.h"
#include "../company_base.h"

#include "state.h"
#include "action.h"
#include "communication.h"

class RLInterface {
private:
	bool enabled;
	RLCommunication rl_comm;

public:
	RLInterface(const std::string& host, int port);
	~RLInterface();

	void CollectState(CompanyID current_company); // Collects state data from the game world
	void SendState(RLState &state); // Sends the collected state data to the RL server
	void ReceiveAtion();
};

#endif // RL_INTERFACE_H