#ifndef RL_STATE_H
#define RL_STATE_H

#include "../stdafx.h"
#include "../tile_type.h"
#include "../map_func.h"
#include "types.h"
#include <vector>

using namespace rl;

struct RLState {
	
	struct CityInfo {
		Location location;
		uint32_t population;
	};

	struct IndustryInfo {
		Location location;
		uint8_t type;
		std::vector<std::pair<uint8_t, uint16_t>> produced_cargos; // cargo type, amount
		std::vector<uint8_t> accepted_cargos; // cargo type indices
	};

	std::vector<CityInfo> cities;
	std::vector<IndustryInfo> industries;
	int64_t current_money; 
	// int current_year;

	std::vector<float> ToVector() const;
	void FromVector(const std::vector<float>& data);
};

#endif // RL_STATE_H