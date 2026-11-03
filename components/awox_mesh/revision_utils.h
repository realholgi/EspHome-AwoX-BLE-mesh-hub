#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace esphome {
namespace awox_mesh {

// Device Information revision characteristics are typically short, printable
// ASCII strings (optionally NUL-terminated). Never interpret BLE bytes as an
// unbounded C string or publish arbitrary binary data to MQTT.
inline std::string parse_revision_string(const uint8_t *data, size_t length) {
  if (data == nullptr || length == 0 || length > 64) return {};
  std::string value;
  for (size_t i = 0; i < length && data[i] != 0; ++i) {
    if (data[i] < 0x20 || data[i] > 0x7e) return {};
    value.push_back(static_cast<char>(data[i]));
  }
  while (!value.empty() && value.back() == ' ') value.pop_back();
  return value;
}

}  // namespace awox_mesh
}  // namespace esphome
