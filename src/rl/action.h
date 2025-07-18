#ifndef RL_ACTION_H
#define RL_ACTION_H

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

enum class RLActionCategory : uint8_t {
	None = 0,
	BuildInfrastructure,  // Roads, rails, bridges, tunnels
	BuildStations,        // All types of stations
	ManageVehicles,       // Buy, sell, send vehicles
	ManageOrders         // Vehicle orders and routes
};

// Separate action enums for each category
enum class BuildInfrastructureAction : uint8_t {
	None = 0,
	BuildRoad,
	BuildRoadLine,
	BuildRoadDepot,
	BuildRail,
	BuildRailLine,
	BuildBridge,
	BuildTunnel,
	RemoveRoad,
	RemoveRail
};

enum class BuildStationsAction : uint8_t {
	None = 0,
	BuildCargoStation,
	BuildBusStation,
	BuildRailStation,
	BuildAirport,
	RemoveStation
};

enum class ManageVehiclesAction : uint8_t {
	None = 0,
	BuyRoadVehicle,
	BuyTrain,
	BuyAircraft,
	BuyShip,
	SellVehicle,
	SendVehicleToDepot,
	StartVehicle,
	StopVehicle
};

enum class ManageOrdersAction : uint8_t {
	None = 0,
	AddOrder,
	RemoveOrder,
	ModifyOrder,
	ClearOrders,
	CloneOrders,
	ShareOrders
};

struct RLAction {
    RLActionCategory category;
    uint8_t type; 
    std::vector<uint32_t> params; // Variable length parameters for the action
    
    // Parameter meanings by category and action:
    // BuildInfrastructure:
    //   BuildRoad: param1=tile, param2=direction
    //   BuildRoadLine: param1=start_tile, param2=end_tile, param3=axis
    // BuildStations:
    //   BuildCargoStation: param1=tile, param2=is_drive_through, param3=direction
    //   BuildBusStation: param1=tile, param2=direction
    // ManageVehicles:
    //   BuyRoadVehicle: param1=depot_tile, param2=vehicle_type
    //   SendVehicleToDepot: param1=vehicle_id
    // ManageOrders:
    //   AddOrder: param1=vehicle_id, param2=destination_tile, param3=order_flags

	void fromBytes(const uint8_t* data, size_t size);
	std::vector<uint8_t> toBytes() const; 
	size_t GetSerializedSize() const;
	
	// Helper methods to get typed actions
	BuildInfrastructureAction GetBuildInfrastructureAction() const {
		return static_cast<BuildInfrastructureAction>(type);
	}
	
	BuildStationsAction GetBuildStationsAction() const {
		return static_cast<BuildStationsAction>(type);
	}
	
	ManageVehiclesAction GetManageVehiclesAction() const {
		return static_cast<ManageVehiclesAction>(type);
	}
	
	ManageOrdersAction GetManageOrdersAction() const {
		return static_cast<ManageOrdersAction>(type);
	}
};

#endif // RL_ACTION_H