#include "components/awox_mesh/protocol_safety.h"
#include <cassert>

using namespace esphome::awox_mesh;

static_assert(!valid_mesh_frame_length(0), "Empty frames are invalid");
static_assert(!valid_mesh_frame_length(19), "Truncated frames are invalid");
static_assert(valid_mesh_frame_length(20), "AwoX frame length");
static_assert(!valid_mesh_frame_length(21), "Excess frame length is invalid");
static_assert(valid_command_payload_length(0), "Empty command payload");
static_assert(valid_command_payload_length(10), "Maximum command payload");
static_assert(!valid_command_payload_length(11), "Oversized command payload");

int main() {
  assert(valid_mesh_frame_length(20));
  assert(!valid_command_payload_length(256));
}
