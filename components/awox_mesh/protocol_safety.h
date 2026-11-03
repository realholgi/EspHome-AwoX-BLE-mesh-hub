#pragma once
#include <cstddef>

namespace esphome {
namespace awox_mesh {

// AwoX mesh frames are 20 bytes; the packet cipher uses a single
// 16-byte AES block and cannot safely handle arbitrary input lengths.
constexpr bool valid_mesh_frame_length(std::size_t size) { return size == 20; }

// 20-byte frame: 10 bytes of header + up to 10 bytes of command data.
constexpr bool valid_command_payload_length(std::size_t size) { return size <= 10; }

}  // namespace awox_mesh
}  // namespace esphome
