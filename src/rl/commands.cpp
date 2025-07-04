#include "commands.h"
#include <cstdint>
#include <cstddef>
#include <optional>
#include <compare>

bool BuildRoadAtTile(int _tile, int _road_type, int _dir)
{
	TileIndex tile_index = TileIndex(_tile);


    return false;
}

bool BuildRoadLine(int _start, int _end, int _axis)
{
	TileIndex start_tile = TileIndex(_start);
	TileIndex end_tile = TileIndex(_end);
	Axis axis = Axis(_axis);

	// Check if the start and end tiles are valid for a straight road line
	// if (start_tile == end_tile) {
	// 	// If both tiles are the same, we can build a road at that tile.
	// 	return BuildRoadAtTile(_start, ROADTYPE_ROAD);
	// }

	int start_x = TileX(start_tile);
    int start_y = TileY(start_tile);
    int end_x = TileX(end_tile);
    int end_y = TileY(end_tile);

	if (start_x == end_x || start_y == end_y) {
        // Valid straight line
		TileIndex start_index = TileIndex(start_tile);
		TileIndex end_index = TileIndex(end_tile);
		DoCommandFlags flags{};
		flags.Set(DoCommandFlag::Execute);
		CommandCost cost = CmdBuildLongRoad(
			flags, // flags
			end_index, // end tile
			start_index, // start tile
			ROADTYPE_ROAD, // road type
			axis, // axis
			DRD_NONE, // no disallowed directions
			false, //
			false, //
			1 // ai
		);
		std::cout << "BuildRoadLine: " << cost.GetCost() << std::endl;

        return true;
    }
    return false;
}

bool BuildStationAtTile(int _tile, int _is_drive_trough, int _dir)
{
    TileIndex tile_index = TileIndex(_tile);
	DiagDirection ddir = static_cast<DiagDirection>(_dir);
	DoCommandFlags flags{};
	flags.Set(DoCommandFlag::Execute);
	CommandCost cost = CmdBuildRoadStop(
		flags, // flags
		tile_index,
		1, 1, // width, length
		RoadStopType::Truck, // always use Bus type
		_is_drive_trough != 0, // is_drive_through
		ddir, // entrance direction (default)
		ROADTYPE_ROAD, // always use ROADTYPE_ROAD
		RoadStopClassID::ROADSTOP_CLASS_DFLT, // no special class
		0, // no special index
		NEW_STATION, // new station
		false // not adjacent
	);
	if (cost.Succeeded()) {
		// If the command was successful, we can assume the station was built.
		return true;
	}
	// If the command was not successful, we return false.
    return false;
}

bool BuildBusStationAtTile(int _tile, int _station_type)
{
    return false;
}

bool BuyVehicle(int _vehicle_type, int _tile)
{
    return false;
}

bool SendVehicle(int _vehicle_id, int _destination_tile)
{
    return false;
}

