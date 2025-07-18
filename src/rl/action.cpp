#include "action.h"

void RLAction::fromBytes(const uint8_t* data, size_t size) {
    size_t offset = 0;

    // Check there is enough data for category + type + param count
    if (size < sizeof(category) + sizeof(type) + sizeof(uint32_t)) {
        throw std::runtime_error("Not enough data for RLAction header");
    }

    // Deserialize category
    std::memcpy(&category, data + offset, sizeof(category));
    offset += sizeof(category);

    // Deserialize type
    std::memcpy(&type, data + offset, sizeof(type));
    offset += sizeof(type);

    // Deserialize param count
    uint32_t param_count = 0;
    std::memcpy(&param_count, data + offset, sizeof(param_count));
    offset += sizeof(param_count);

    // Check there is enough data for all params
    size_t expected_size = offset + param_count * sizeof(uint32_t);
    if (size < expected_size) {
        throw std::runtime_error("Not enough data for RLAction parameters");
    }

    // Deserialize params
    params.resize(param_count);
    std::memcpy(params.data(), data + offset, param_count * sizeof(uint32_t));
}

std::vector<uint8_t> RLAction::toBytes() const {
    std::vector<uint8_t> buffer;

    // Calculate total size
    size_t total_size = sizeof(category) + sizeof(type) + sizeof(uint32_t) + params.size() * sizeof(uint32_t);
    buffer.resize(total_size);

    size_t offset = 0;

    // Serialize category
    std::memcpy(buffer.data() + offset, &category, sizeof(category));
    offset += sizeof(category);

    // Serialize type
    std::memcpy(buffer.data() + offset, &type, sizeof(type));
    offset += sizeof(type);

    // Serialize param count
    uint32_t param_count = static_cast<uint32_t>(params.size());
    std::memcpy(buffer.data() + offset, &param_count, sizeof(param_count));
    offset += sizeof(param_count);

    // Serialize params
    std::memcpy(buffer.data() + offset, params.data(), param_count * sizeof(uint32_t));

    return buffer;
}

size_t RLAction::GetSerializedSize() const {
    return sizeof(category) + sizeof(type) + sizeof(uint32_t) + params.size() * sizeof(uint32_t);
}
