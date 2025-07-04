#ifndef RL_COMMANDS_H
#define RL_COMMANDS_H

#include "../stdafx.h"
#include "../command_type.h"
#include "../road_cmd.h"
#include "../station_cmd.h"
#include "../vehicle_cmd.h"
#include "../order_cmd.h"
#include "../newgrf_roadstop.h"

#include <iostream>



bool BuildRoadAtTile(int _tile, int _road_type, int _dir);
bool BuildRoadLine(int _start, int _end, int _axis);
bool BuildStationAtTile(int _tile, int _is_drive_trough, int _dir);
bool BuyVehicle(int _vehicle_type, int _tile);
bool SendVehicle(int _vehicle_id, int _destination_tile);


#endif // RL_INTERFACE_H