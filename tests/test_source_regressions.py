"""Regression checks for connection ownership and duplicate MAC selection.

These structural tests accompany the native frame tests; actual BLE scheduling
and connection races must also be tested on hardware.
"""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
MESH_H = (ROOT / "components/awox_mesh/awox_mesh.h").read_text()
MESH_CPP = (ROOT / "components/awox_mesh/awox_mesh.cpp").read_text()
CONN_H = (ROOT / "components/awox_mesh/mesh_connection.h").read_text()
CONN_CPP = (ROOT / "components/awox_mesh/mesh_connection.cpp").read_text()


class ConnectionRegressions(unittest.TestCase):
    def test_initial_state(self):
        self.assertIn("int mesh_id{0};", MESH_H)
        self.assertIn("FoundDevice *found_device{nullptr};", CONN_H)
        for name in ("notification_char", "command_char", "pair_char"):
            self.assertIn(f"*{name}{{nullptr}};", CONN_H)

    def test_no_duplicate_mac_allocation(self):
        self.assertIn("const bool already_reserved = std::any_of(", MESH_CPP)
        self.assertIn("connection->get_address() == address", MESH_CPP)
        self.assertIn("!found_device->connected && !already_reserved", MESH_CPP)

    def test_release_reservation(self):
        self.assertIn("this->found_device->connected = false;", CONN_CPP)
        self.assertIn("this->found_device = nullptr;", CONN_CPP)

    def test_overlapping_mesh_ids_check_all_pairs(self):
        self.assertIn("for (int j = i + 1; j < connections_count; ++j)", MESH_CPP)
    def test_command_queue_has_limits_and_retries(self):
        self.assertIn("MAX_QUEUED_COMMANDS = 32", CONN_H)
        self.assertIn("MAX_COMMAND_ATTEMPTS = 3", CONN_H)
        self.assertIn("if (this->write_command(item.command, item.data, item.dest, false))", CONN_CPP)
        self.assertIn("if (++item.attempts >= MAX_COMMAND_ATTEMPTS)", CONN_CPP)
        self.assertIn("this->command_queue.clear();", CONN_CPP)
        self.assertIn("this->command_queue.size() >= MAX_QUEUED_COMMANDS", CONN_CPP)


if __name__ == "__main__":
    unittest.main()
