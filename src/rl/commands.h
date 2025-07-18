#ifndef RL_COMMANDS_H
#define RL_COMMANDS_H

#include "../stdafx.h"
#include "../command_type.h"
#include "../road_cmd.h"
#include "../station_cmd.h"
#include "../vehicle_cmd.h"
#include "../order_cmd.h"
#include "../newgrf_roadstop.h"
#include "types.h"

#include <iostream>

using namespace rl;

namespace cmd {

bool BuildRoadAtTile(Location _tile, uint32_t _dir);
bool BuildRoadLine(Location _start, Location _end, uint32_t _axis);
bool BuildRoadDepot(Location _tile, uint32_t _dir);
bool BuildCargoStationAtTile(Location _tile, bool _is_drive_through, uint32_t _dir);
bool BuildBusStationAtTile(Location _tile, bool _is_drive_through, uint32_t _dir);
bool BuyVehicle(uint32_t _vehicle_type, Location _tile);
bool SendVehicle(uint32_t _vehicle_id, Location _destination_tile);

}


#endif // RL_INTERFACE_H