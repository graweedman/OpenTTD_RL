#include "commands.h"
#include <cstdint>
#include <cstddef>
#include <optional>
#include <compare>

namespace cmd {


bool BuildRoadAtTile(Location _tile, uint32_t _dir)
{
	TileIndex tile = TileXY(_tile.x, _tile.y);
	DoCommandFlags flags{};
	flags.Set(DoCommandFlag::Execute);
	CommandCost cost = CmdBuildRoad(
		flags,
		tile,
		ROAD_ALL,
		ROADTYPE_ROAD,
		DRD_NONE,
		TownID::Invalid()
	);
	return cost.Succeeded();
}

bool BuildRoadLine(Location _start, Location _end, uint32_t _axis)
{
	TileIndex start_tile = TileXY(_start.x, _start.y);
	TileIndex end_tile = TileXY(_end.x, _end.y);
	Axis axis = Axis(_axis);
	DoCommandFlags flags{};
	flags.Set(DoCommandFlag::Execute);

	if (_start.x == _end.x || _start.y == _end.y) {
        // Valid straight line
		CommandCost cost = CmdBuildLongRoad(
			flags,
			end_tile,
			start_tile,
			ROADTYPE_ROAD,
			axis,
			DRD_NONE,
			false, // start_half
			false, // end_half
			true // ai
		);
		std::cout << "BuildRoadLine: " << cost.GetCost() << std::endl;
		return cost.Succeeded();
    }
	return false; // Invalid line, not straight
}

bool BuildRoadDepot(Location _tile, uint32_t _dir)
{
	TileIndex tile = TileXY(_tile.x, _tile.y);
	DiagDirection ddir = static_cast<DiagDirection>(_dir);
	DoCommandFlags flags{};
	flags.Set(DoCommandFlag::Execute);
	CommandCost cost = CmdBuildRoadDepot(
		flags,
		tile,
		ROADTYPE_ROAD, // always use Road type
		ddir
	);
	return cost.Succeeded();
}

bool BuildCargoStationAtTile(Location _tile, bool _is_drive_through, uint32_t _dir)
{
    TileIndex tile = TileXY(_tile.x, _tile.y);
	DiagDirection ddir = static_cast<DiagDirection>(_dir);
	DoCommandFlags flags{};
	flags.Set(DoCommandFlag::Execute);
	CommandCost cost = CmdBuildRoadStop(
		flags,
		tile,
		1, 1, // width, length
		RoadStopType::Truck, // always use Truck type
		_is_drive_through,
		ddir,
		ROADTYPE_ROAD,
		RoadStopClassID::ROADSTOP_CLASS_DFLT,
		0, // special index
		NEW_STATION,
		false // adjacent
	);
	return cost.Succeeded();
}

bool BuildBusStationAtTile(Location _tile, bool _is_drive_through, uint32_t _dir)
{
    TileIndex tile = TileXY(_tile.x, _tile.y);
	DiagDirection ddir = static_cast<DiagDirection>(_dir);
	DoCommandFlags flags{};
	flags.Set(DoCommandFlag::Execute);
	CommandCost cost = CmdBuildRoadStop(
		flags,
		tile,
		1, 1, // width, length
		RoadStopType::Bus, // always use Bus type
		_is_drive_through,
		ddir,
		ROADTYPE_ROAD,
		RoadStopClassID::ROADSTOP_CLASS_DFLT,
		0, // special index
		NEW_STATION,
		false // adjacent
	);
	return cost.Succeeded();
}

bool BuyVehicle(uint32_t _vehicle_type, Location _tile)
{
    return false;
}

bool SendVehicle(uint32_t _vehicle_id, Location _destination_tile)
{
    return false;
}

}

