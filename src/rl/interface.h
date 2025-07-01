#ifndef RL_INTERFACE_H
#define RL_INTERFACE_H

#include "../stdafx.h"
#include "state.h"
#include "action.h"

class RLInterface {
private:
	bool enabled;

public:
	RLInterface();
	~RLInterface();

	void CollectState();
	void SendData();
	void ReceiveAtion();
};

#endif // RL_INTERFACE_H