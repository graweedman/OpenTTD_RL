#include "interface.h"

RLInterface::RLInterface(const std::string& host, int port) {
	if (rl_comm.Connect(host, port)) {
		enabled = true;
	} else {
		enabled = false;
		std::cerr << "Failed to connect to RL server at " << host << ":" << port << std::endl;
	}
}

RLInterface::~RLInterface() {
	rl_comm.Close();
	enabled = false;
	std::cout << "RLInterface disconnected." << std::endl;
}

void RLInterface::CollectState(CompanyID company_id) {
	if (!enabled) return;

	RLState state;
	// Collect data from the game world
	// Example: collect town and industry data
	for (Town *t : Town::Iterate()) {
		RLState::CityInfo city_info = { t->xy, t->cache.population };
		state.cities.push_back(city_info);
	}
	for (Industry *i : Industry::Iterate()) {
		TileIndex tile = i->location.tile;
		uint width = i->location.w;
		uint height = i->location.h;
		RLState::IndustryInfo info{ tile, i->type };

		for (const Industry::ProducedCargo &prod : i->produced) {
			if (prod.rate) info.produced_cargos.emplace_back(prod.cargo, prod.rate);
		}
		for (const Industry::AcceptedCargo &acc : i->accepted) {
			if (acc.waiting) info.accepted_cargos.push_back(acc.cargo); // or acc.cargo_type
		}
		state.industries.push_back(info);
	}
	RLInterface::SendState(state);
	// Company *company = Company::Get(company_id);
	// state.current_money = company->money;
}

void RLInterface::SendState(RLState &state) {
	if (!enabled) return;
	std::vector<float> data;

	data.push_back(static_cast<float>(state.cities.size()));
	data.push_back(static_cast<float>(state.industries.size()));

	std::vector<float> state_data = state.ToVector();
	data.insert(data.end(), state_data.begin(), state_data.end());
	
	rl_comm.Send(data);
}

void RLInterface::ReceiveAction() {
	if (!enabled) return;
	RLAction action;
	std::vector<uint8_t> data;

	if (rl_comm.Receive(data, 128)) {
		if (data.size() < sizeof(RLAction)) {
			std::cerr << "Received invalid action data size: " << data.size() << std::endl;
			return;
		}
		
		action.fromBytes(data.data(), data.size());
		RLInterface::ExecuteAction(action);
	}
}

void RLInterface::ExecuteAction(const RLAction &action)
{
	if (!enabled) return;

	bool success = false;

	// Example: handle a build road action
	switch (action.type) {
		case RLActionType::BuildRoad:
			if (action.param1 < 0 || action.param2 < 0) {
				std::cerr << "Invalid parameters for BuildRoad action: " << action.param1 << ", " << action.param2 << std::endl;
				return;
			}
			// BuildRoadAtTile(action.param1, action.param2); // tile, direction, length/type
			break;
		case RLActionType::BuildRoadLine:
			if (action.param1 < 0 || action.param2 < 0 || action.param3 < 0) {
				std::cerr << "Invalid parameters for BuildRoadLine action: " << action.param1 << ", " << action.param2 << ", " << action.param3 << std::endl;
				return;
			}
			success = BuildRoadLine(action.param1, action.param2, action.param3); // start_tile, end_tile, axis
			break;
		case RLActionType::BuildCargoStation:
			if (action.param1 < 0 || action.param2 < 0 || action.param3 < 0) {
				std::cerr << "Invalid parameters for BuildStation action: " << action.param1 << ", " << action.param2 << std::endl;
				return;
			}
			std::cout << "Building cargo station at tile: " << action.param1 << ", is_drive_through: " << action.param2 << ", direction: " << action.param3 << std::endl;
			success = BuildStationAtTile(action.param1, action.param2, action.param3); // tile, is_drive_trough, direction
			break;
		case RLActionType::BuyVehicle:
			// CmdBuyVehicle(DoCommandFlag::Execute, action.param1, action.param2);
			break;
		case RLActionType::SendVehicle:
			// CmdSendVehicle(DoCommandFlag::Execute, action.param1, action.param2);
			break;
		default:
			std::cerr << "Unknown action type: " << static_cast<int>(action.type) << std::endl;
			return;
	}

	std::vector<float> result_msg = { success ? 1.0f : 0.0f }; // Assume success for now
	rl_comm.Send(result_msg);
}
