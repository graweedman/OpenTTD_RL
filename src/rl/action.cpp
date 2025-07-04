#include "action.h"

void RLAction::fromBytes(const uint8_t* data, size_t size) {
	if (size < sizeof(RLAction)) return; // Handle error or throw
	std::memcpy(this, data, sizeof(RLAction));
}