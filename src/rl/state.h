#ifndef RL_STATE_H
#define RL_STATE_H

#include "../stdafx.h"
#include <vector>

struct State {
	struct CityInfo {
		int x, y;
		int population;
	};

	struct IndustryInfo {
		int x, y;
		int type;
		int production;
	};

	std::vector<CityInfo> cities;
	std::vector<IndustryInfo> industries;
	int current_money;
	int current_year;

	std::vector<float> ToVector() const;
	void FromVector(const std::vector<float>& data);
};

#endif // RL_STATE_H