#include "state.h"

std::vector<float> RLState::ToVector() const
{
    std::vector<float> data;
    for (const auto& city : cities) {
		uint32_t x = city.location.x;
		uint32_t y = city.location.y;
        data.push_back(static_cast<float>(x));
		data.push_back(static_cast<float>(y));
        data.push_back(static_cast<float>(city.population));
    }
    for (const auto& industry : industries) {
		uint32_t x = industry.location.x;
		uint32_t y = industry.location.y;
		data.push_back(static_cast<float>(x));
		data.push_back(static_cast<float>(y));
        data.push_back(static_cast<float>(industry.type));
		data.push_back(static_cast<float>(industry.produced_cargos.size())); // number of produced cargos
		for (const auto& cargo : industry.produced_cargos) {
			data.push_back(static_cast<float>(cargo.first)); // cargo type
			data.push_back(static_cast<float>(cargo.second)); // amount
		}
		data.push_back(static_cast<float>(industry.accepted_cargos.size())); // number of accepted cargos
		for (const auto& cargo : industry.accepted_cargos) {
			data.push_back(static_cast<float>(cargo)); // cargo type
		}
    }
    data.push_back(static_cast<float>(current_money));
    //data.push_back(static_cast<float>(current_year));
    return data;
}

void RLState::FromVector(const std::vector<float> &data)
{
	// size_t index = 0;
	// for (auto& city : cities) {
	// 	city.x = static_cast<int>(data[index++]);
	// 	city.y = static_cast<int>(data[index++]);
	// 	city.population = static_cast<int>(data[index++]);
	// }
	// for (auto& industry : industries) {
	// 	industry.location = static_cast<int>(data[index++]);
	// 	industry.type = static_cast<int>(data[index++]);
	// 	industry.production = static_cast<int>(data[index++]);
	// }
	// current_money = static_cast<int>(data[index++]);
	// current_year = static_cast<int>(data[index++]);
}
