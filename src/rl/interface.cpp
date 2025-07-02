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
	// for (Town *t : Town::Iterate()) {
	// 	RLState::CityInfo city_info = { t->x, t->y, t->population };
	// 	state.cities.push_back(city_info);
	// }
	for (Industry *i : Industry::Iterate()) {
		TileIndex tile = i->location.tile;
		uint width = i->location.w;
		uint height = i->location.h;

		for (const Industry::ProducedCargo & prod : i->produced) {
			if (prod.rate > 0) {
				// Only include industries that produce cargo
				RLState::IndustryInfo industry_info = { tile, i->type, prod.rate };
				state.industries.push_back(industry_info);
			}
		}
	}
	// Company *company = Company::Get(company_id);
	// state.current_money = company->money;
}

void RLInterface::SendState(RLState &state) {
	if (!enabled) return;

	std::vector<float> data = state.ToVector();
	rl_comm.Send(std::vector<uint8_t>(reinterpret_cast<uint8_t*>(data.data()), reinterpret_cast<uint8_t*>(data.data() + data.size())));
}
