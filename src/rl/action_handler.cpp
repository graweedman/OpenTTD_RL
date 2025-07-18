#include "action_handler.h"

bool RLActionHandler::HandleNoneAction(const RLAction &action)
{
    return false;
}

bool RLActionHandler::HandleAction(const RLAction &action)
{
	std::cout << "Handling action of category: " << static_cast<int>(action.category) << std::endl;
	switch (action.category) {
		case RLActionCategory::BuildInfrastructure:
			return HandleBuildInfrastructure(action);
		case RLActionCategory::BuildStations:
			std::cout << "Handling BuildStations action" << std::endl;
			return HandleBuildStations(action);
		case RLActionCategory::ManageVehicles:
			return false;
			// return HandleManageVehicles(action);
		case RLActionCategory::ManageOrders:
			return false;
			// return HandleManageOrders(action);
		default:
			return HandleNoneAction(action);
	}
    return false;
}

bool RLActionHandler::HandleBuildInfrastructure(const RLAction &action)
{
	switch (action.GetBuildInfrastructureAction()) {
		case BuildInfrastructureAction::BuildRoad: {
			Location tile = { action.params[0], action.params[1] };
			uint32_t direction = action.params[2];
			return cmd::BuildRoadAtTile(tile, direction);
		}
			
		case BuildInfrastructureAction::BuildRoadLine: {
			Location start_tile = { action.params[0], action.params[1] };
			Location end_tile = { action.params[2], action.params[3] };
			return cmd::BuildRoadLine(start_tile, end_tile, action.params[4]);
		}

		case BuildInfrastructureAction::BuildRoadDepot: {
			Location tile = { action.params[0], action.params[1] };
			uint32_t direction = action.params[2];
			return cmd::BuildRoadDepot(tile, direction);
		}

		default:
			return HandleNoneAction(action);
	}
}

bool RLActionHandler::HandleBuildStations(const RLAction &action)
{
	switch (action.GetBuildStationsAction()) {
		case BuildStationsAction::BuildCargoStation: {
			Location tile = { action.params[0], action.params[1] };
			bool is_drive_through = action.params[2] != 0;
			uint32_t direction = action.params[3];
			return cmd::BuildCargoStationAtTile(tile, is_drive_through, direction);
		}

		case BuildStationsAction::BuildBusStation: {
			Location tile = { action.params[0], action.params[1] };
			bool is_drive_through = action.params[2] != 0;
			uint32_t direction = action.params[3];
			return cmd::BuildBusStationAtTile(tile, is_drive_through, direction);
		}

		default:
			return HandleNoneAction(action);
	}
}