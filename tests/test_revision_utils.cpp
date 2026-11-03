#include "components/awox_mesh/revision_utils.h"
#include <cassert>
#include <cstdint>
#include <string>

using esphome::awox_mesh::parse_revision_string;

int main() {
  const uint8_t firmware[] = {'1', '.', '2', '.', '3', 0};
  assert(parse_revision_string(firmware, sizeof(firmware)) == "1.2.3");
  const uint8_t hardware[] = {'H', 'W', '-', '2', ' ', ' '};
  assert(parse_revision_string(hardware, sizeof(hardware)) == "HW-2");
  const uint8_t invalid[] = {'1', '\n', '2'};
  assert(parse_revision_string(invalid, sizeof(invalid)).empty());
  assert(parse_revision_string(nullptr, 0).empty());
  const uint8_t long_value[65]{};
  assert(parse_revision_string(long_value, sizeof(long_value)).empty());
  const uint8_t blank[] = {0};
  assert(parse_revision_string(blank, sizeof(blank)).empty());
}
