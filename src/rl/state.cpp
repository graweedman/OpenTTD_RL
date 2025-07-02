#include "state.h"

std::vector<float> RLState::ToVector() const
{
    // std::vector<float> data;
    // for (const auto& city : cities) {
    //     data.push_back(static_cast<float>(city.x));
    //     data.push_back(static_cast<float>(city.y));
    //     data.push_back(static_cast<float>(city.population));
    // }
    // for (const auto& industry : industries) {
    //     data.push_back(static_cast<float>(industry.location));
    //     data.push_back(static_cast<float>(industry.type));
    //     data.push_back(static_cast<float>(industry.production));
    // }
    // data.push_back(static_cast<float>(current_money));
    // data.push_back(static_cast<float>(current_year));
    return {};
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
