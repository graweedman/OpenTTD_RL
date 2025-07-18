#ifndef RL_INTERFACE_H
#define RL_INTERFACE_H

#include "../stdafx.h"
#include "../town.h"
#include "../industry.h"
#include "../company_base.h"

#include "state.h"
#include "action.h"
#include "communication.h"
#include "commands.h"
#include "action_handler.h"
#include "types.h"

using namespace rl;

class RLInterface {
private:
	bool enabled;
	RLCommunication rl_comm;

public:
	RLInterface(const std::string& host, int remote_port, int local_port = 0);
	~RLInterface();

	void CollectState(CompanyID current_company); // Collects state data from the game world
	void SendState(RLState &state); // Sends the collected state data to the RL server
	void ReceiveAction();
	void ExecuteAction(const RLAction &action); // Executes an action received from the RL server
};

#endif // RL_INTERFACE_H