#include "interface.h"

RLInterface::RLInterface(const std::string& host, int remote_port, int local_port) {
	if (rl_comm.Connect(host, remote_port, local_port)) {
		enabled = true;
	} else {
		enabled = false;
		std::cerr << "Failed to connect to RL server at " << host << ":" << remote_port << std::endl;
	}
}

RLInterface::~RLInterface() {
	rl_comm.Close();
	enabled = false;
	std::cout << "RLInterface disconnected." << std::endl;
}

void RLInterface::CollectState(CompanyID company_id ) {
	if (!enabled) return;

	RLState state;
	// Collect data from the game world
	// Example: collect town and industry data
	for (Town *town : Town::Iterate()) {
		Location tile = { TileX(town->xy), TileY(town->xy) };
		RLState::CityInfo city_info = { tile , town->cache.population };
		state.cities.push_back(city_info);
	}
	for (Industry *industry : Industry::Iterate()) {
		Location tile = { TileX(industry->location.tile), TileY(industry->location.tile) };
		RLState::IndustryInfo info{ tile, industry->type, {}, {} };

		for (const Industry::ProducedCargo &prod : industry->produced) {
			if (prod.rate) info.produced_cargos.emplace_back(prod.cargo, prod.rate);
		}
		for (const Industry::AcceptedCargo &acc : industry->accepted) {
			if (acc.waiting) info.accepted_cargos.push_back(acc.cargo); // or acc.cargo_type
		}
		state.industries.push_back(info);
	}
	Company *company = Company::Get(company_id);
	state.current_money = company->money;
	RLInterface::SendState(state);
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
		// Minimum size: category (1) + type (1) + param_count (4) = 6 bytes
		const size_t min_size = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t);
		if (data.size() < min_size) {
			std::cerr << "Received invalid action data size: " << data.size() << ", minimum required: " << min_size << std::endl;
			return;
		}
		std::cout << "Received action data of size: " << data.size() << std::endl;
		try {
			action.fromBytes(data.data(), data.size());
			std::cout << "Successfully parsed action, category: " << static_cast<int>(action.category) << std::endl;
			RLInterface::ExecuteAction(action);
		} catch (const std::exception& e) {
			std::cerr << "Error parsing action: " << e.what() << std::endl;
		}
	}
}

void RLInterface::ExecuteAction(const RLAction &action)
{
	if (!enabled) return;

	bool success = false;
	if (action.category == RLActionCategory::None) {
		std::cout << "Received None action, ignoring." << std::endl;
		return;
	}
	std::cout << "Executing action of category: " << static_cast<int>(action.category) << std::endl;
	success = RLActionHandler::HandleAction(action);

	std::vector<float> result_msg = { success ? 1.0f : 0.0f }; // Assume success for now
	rl_comm.Send(result_msg);
}
