#ifndef RL_ACTION_H
#define RL_CCTION_H

#include "../stdafx.h"

enum class RLActionType : uint8_t {
    None = 0,
    BuildRoad,
	BuildRoadLine,
    BuildCargoStation,
	BuildBusStation,
    BuyVehicle,
    SendVehicle,
    // Add more actions as needed
};

struct RLAction {
    RLActionType type;
    int param1; // e.g., tile index or entity id
    int param2; // e.g., direction or vehicle type
	int param3;
	int param4;
	int param5;
    // Add more parameters as needed

	void fromBytes(const uint8_t* data, size_t size);
};

#endif // RL_ACTION_H