#ifndef RL_STATE_H
#define RL_STATE_H

#include "../stdafx.h"
#include "../tile_type.h"
#include <vector>

struct RLState {
	struct CityInfo {
		TileIndex location;
		int population;
	};

	struct IndustryInfo {
		TileIndex location;
		int type;
		std::vector<std::pair<int, int>> produced_cargos; // cargo type indices produced
		std::vector<int> accepted_cargos; // cargo type indices accepted
	};

	std::vector<CityInfo> cities;
	std::vector<IndustryInfo> industries;
	int current_money;
	// int current_year;

	std::vector<float> ToVector() const;
	void FromVector(const std::vector<float>& data);
};

#endif // RL_STATE_H