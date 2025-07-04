#include "state.h"

std::vector<float> RLState::ToVector() const
{
    std::vector<float> data;
    for (const auto& city : cities) {
        data.push_back(static_cast<float>(city.location.value));
        data.push_back(static_cast<float>(city.population));
    }
    for (const auto& industry : industries) {
		float tile_as_float = static_cast<float>(industry.location.value);
        data.push_back(tile_as_float);
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
    // data.push_back(static_cast<float>(current_money));
    // data.push_back(static_cast<float>(current_year));
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
