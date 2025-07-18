#ifndef RL_ACTION_HANDLER_H
#define RL_ACTION_HANDLER_H

#include "action.h"
#include "types.h"
#include "commands.h"
#include "../map_func.h"

using namespace rl;

class RLActionHandler {

private:
	// Infrastructure
	static bool HandleBuildInfrastructure(const RLAction& action);

	// Stations
	static bool HandleBuildStations(const RLAction& action);


	// Vehicles
	// static bool HandleManageVehicles(const RLAction& action);

	// static bool BuyVehicle(Location depot_tile, uint32_t vehicle_type);
	// static bool SendVehicle(uint32_t vehicle_id, uint32_t destination);
	// static bool AddVehicleOrder(uint32_t vehicle_id, uint32_t destination_tile, uint32_t order_flags);
	// static bool StartStopVehicle(uint32_t vehicle_id, bool start);
	// static bool SellVehicle(uint32_t vehicle_id);

	
	// static bool HandleManageOrders(const RLAction& action);



	// Add more private methods for other action categories as needed
	static bool HandleNoneAction(const RLAction& action); // Handle no action or invalid action
	// static bool HandleUnknownAction(const RLAction& action); // Handle unknown action types

	
    
    
	
public:
	static bool HandleAction(const RLAction& action); 
};


#endif // RL_ACTION_HANDLER_H