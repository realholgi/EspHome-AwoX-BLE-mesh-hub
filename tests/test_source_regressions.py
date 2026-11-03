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

    def test_same_mac_does_not_clear_found_device(self):
        # In ESPHome 2026.10, BLEClientBase::parse_device calls the virtual
        # set_address() with the existing MAC. We must preserve ownership.
        method = CONN_CPP.split("void MeshConnection::set_address(uint64_t address) {", 1)[1].split(
            "void MeshConnection::loop()", 1
        )[0]
        self.assertIn("if (address == this->get_address()) return;", method)
        self.assertLess(
            method.index("if (address == this->get_address()) return;"),
            method.index("this->found_device = nullptr;"),
        )
        connect = CONN_CPP.split("void MeshConnection::connect_to(FoundDevice *found_device) {", 1)[1].split(
            "void MeshConnection::set_address", 1
        )[0]
        self.assertIn("this->parse_device(found_device->device);", connect)
        self.assertIn("this->found_device->mesh_id", connect)

    def test_overlapping_mesh_ids_check_all_pairs(self):
        self.assertIn("for (int j = i + 1; j < connections_count; ++j)", MESH_CPP)
    def test_command_queue_has_limits_and_retries(self):
        self.assertIn("MAX_QUEUED_COMMANDS = 32", CONN_H)
        self.assertIn("MAX_COMMAND_ATTEMPTS = 3", CONN_H)
        self.assertIn("if (this->write_command(item.command, item.data, item.dest, false))", CONN_CPP)
        self.assertIn("if (++item.attempts >= MAX_COMMAND_ATTEMPTS)", CONN_CPP)
        self.assertIn("this->command_queue.clear();", CONN_CPP)
        self.assertIn("this->command_queue.size() >= MAX_QUEUED_COMMANDS", CONN_CPP)
    def test_mqtt_caches_follow_successful_publish_and_reconnect(self):
        mqtt = (ROOT / "components/awox_mesh/awox_mesh_mqtt.cpp").read_text()
        header = (ROOT / "components/awox_mesh/awox_mesh_mqtt.h").read_text()
        self.assertIn("this->last_published_state_.clear()", mqtt)
        self.assertIn("this->last_published_availability_.clear()", mqtt)
        self.assertIn("this->mesh_->refresh_mqtt_discovery()", mqtt)
        self.assertIn("if (status_published)", mqtt)
        self.assertIn("if (published) this->last_published_state_", mqtt)
        self.assertIn("if (global_mqtt_client->publish(", mqtt)
        self.assertIn("std::set<int> subscribed_destinations_", header)
    def test_rssi_cleanup_is_throttled_and_stale_devices_reclaimed(self):
        self.assertIn("now - this->last_found_device_cleanup >= FOUND_DEVICE_RSSI_TIMEOUT_MS", MESH_CPP)
        self.assertIn("age >= FOUND_DEVICE_RECLAIM_MS", MESH_CPP)
        self.assertIn("!found_device->connected && !reserved", MESH_CPP)
        self.assertIn("delete found_device;", MESH_CPP)
        self.assertIn("FOUND_DEVICE_RECLAIM_MS = 600000", MESH_H)
        discovery = MESH_CPP.split("FoundDevice *AwoxMesh::add_to_found_devices(")[1].split(
            "bool AwoxMesh::parse_device(")[0]
        self.assertNotIn("set_rssi_for_devices_that_are_not_available()", discovery)

    def test_mesh_secrets_not_logged(self):
        self.assertNotIn('ESP_LOGI("awox.mesh", "password:', MESH_H)
        self.assertNotIn('session key %s', CONN_CPP)
        self.assertNotIn('combine mesh name + password:', CONN_CPP)

    def test_untrusted_mesh_packets_are_bounded(self):
        self.assertIn("valid_mesh_frame_length(param->notify.value_len)", CONN_CPP)
        self.assertIn("valid_mesh_frame_length(packet.size())", CONN_CPP)
        self.assertIn("valid_command_payload_length(data.size())", CONN_CPP)
        self.assertIn('this->random_key.assign(reinterpret_cast<const char *>(key), sizeof(key));', CONN_CPP)

    def test_mqtt_reconnect_does_not_duplicate_subscriptions(self):
        mqtt = (ROOT / "components/awox_mesh/awox_mesh_mqtt.cpp").read_text()
        self.assertIn("const bool newly_subscribed = this->subscribed_destinations_", mqtt)
        self.assertIn("this->mesh_->refresh_mqtt_discovery()", mqtt)
        self.assertIn("if (status_published)", mqtt)

    def test_device_info_retry_is_bounded_during_disconnection(self):
        mesh_loop = MESH_CPP.split("void AwoxMesh::loop() {", 1)[1].split(
            "void AwoxMesh::disconnect_connections_with_overlapping_mesh_ids()", 1
        )[0]
        self.assertIn("now - device->device_info_requested >= this->device_info_request_interval_ms", mesh_loop)

        request = MESH_CPP.split("void AwoxMesh::request_device_info(Device *device) {", 1)[1].split(
            "void AwoxMesh::set_power(", 1
        )[0]
        self.assertLess(
            request.index("device->device_info_requested = esphome::millis();"),
            request.index("this->call_connection("),
        )

    def test_mesh_requests_are_only_queued_on_established_ble_connections(self):
        call = MESH_CPP.split("void AwoxMesh::call_connection(", 1)[1].split(
            "void AwoxMesh::request_device_info(", 1
        )[0]
        self.assertIn("connection->connected() && connection->mesh_id_linked(dest)", call)
        self.assertIn("if (connection->connected())", call)
        self.assertNotIn("ESP_LOGI(TAG, \"No active connection", call)

    def test_connection_diagnostics_survive_slot_reuse(self):
        for member in ("connection_attempts_", "connection_failures_", "disconnect_count_",
                       "last_disconnect_reason_", "last_rssi_dbm_", "established_session_"):
            self.assertIn(member, CONN_H)
        self.assertIn("++this->connection_attempts_", CONN_CPP)
        self.assertIn("++this->disconnect_count_", CONN_CPP)
        self.assertIn("++this->connection_failures_", CONN_CPP)
        self.assertIn("this->last_disconnect_reason_ = param->disconnect.reason", CONN_CPP)
        self.assertIn("this->last_rssi_dbm_ = found_device->rssi", CONN_CPP)
        self.assertIn("this->established_session_ = true", CONN_CPP)
        # No change to the same-MAC guard that previously caused a crash.
        self.assertIn("if (address == this->get_address()) return;", CONN_CPP)

    def test_packet_age_and_write_failure_diagnostics(self):
        for marker in ("failed_command_writes_", "dropped_commands_", "has_notification_",
                       "last_notification_ms_", "ms_since_notification"):
            self.assertIn(marker, CONN_H)
        self.assertIn("++this->failed_command_writes_", CONN_CPP)
        self.assertIn("++this->dropped_commands_", CONN_CPP)
        self.assertIn("this->last_notification_ms_ = esphome::millis()", CONN_CPP)
        self.assertIn("if (packet.size() != 20) break;", CONN_CPP)

    def test_connection_diagnostics_are_published_throttled(self):
        mqtt = (ROOT / "components/awox_mesh/awox_mesh_mqtt.cpp").read_text()
        header = (ROOT / "components/awox_mesh/awox_mesh_mqtt.h").read_text()
        for field in ("connection_attempts", "connection_failures", "disconnect_count",
                      "queue_length", "failed_command_writes", "dropped_commands",
                      "rssi", "last_disconnect_reason", "last_notification_age_s"):
            self.assertIn(f'connection["{field}"]', mqtt)
        self.assertIn("now - this->last_diagnostic_publish_ms_ < 30000", mqtt)
        self.assertIn("this->last_diagnostic_publish_ms_ = now", mqtt)
        self.assertIn("last_diagnostic_publish_ms_", header)

    def test_home_assistant_diagnostic_discovery_uses_existing_topic(self):
        mqtt = (ROOT / "components/awox_mesh/awox_mesh_mqtt.cpp").read_text()
        self.assertIn("static void publish_diagnostic_sensor(", mqtt)
        self.assertIn('root[MQTT_ENTITY_CATEGORY] = "diagnostic"', mqtt)
        self.assertIn('root[MQTT_STATE_TOPIC] = global_mqtt_client->get_topic_prefix() + "/connection_status"', mqtt)
        for slug in ("-rssi", "-attempts", "-failures", "-disconnects", "-reason",
                     "-notification-age", "-queue", "-write-errors", "-dropped"):
            self.assertIn('slot + "' + slug + '"', mqtt)
        self.assertIn('"active-connections"', mqtt)
        self.assertIn('"online-devices"', mqtt)

    def test_display_snapshot_is_read_only_and_bounded(self):
        self.assertIn("struct GatewayDisplayStatus", MESH_H)
        self.assertIn("GatewayDisplayStatus get_display_status() const;", MESH_H)
        body = MESH_CPP.split("GatewayDisplayStatus AwoxMesh::get_display_status() const {", 1)[1].split(
            "void AwoxMesh::publish_connected()", 1
        )[0]
        self.assertIn("selected->last_rssi_dbm()", body)
        self.assertIn("selected->ms_since_notification(now)", body)
        self.assertIn("device->device_info->has_feature(FEATURE_LIGHT_MODE)", body)
        self.assertNotIn("request_status_update(", body)
        self.assertNotIn("call_connection(", body)
        self.assertNotIn("global_mqtt_client", body)

    def test_optional_gatt_revision_reads_follow_mesh_authentication(self):
        self.assertIn("this->get_characteristic(0x180A, 0x2A26)", CONN_CPP)
        self.assertIn("this->get_characteristic(0x180A, 0x2A27)", CONN_CPP)
        self.assertIn("this->mesh_authenticated_ = true;", CONN_CPP)
        self.assertIn("this->request_next_revision_();", CONN_CPP)
        self.assertIn("this->pending_revision_handle_ = 0;", CONN_CPP)
        self.assertIn("this->mesh_->set_device_revision(this->get_address(), is_firmware, revision)", CONN_CPP)
        self.assertIn("parse_revision_string(param->read.value, param->read.value_len)", CONN_CPP)
        self.assertIn("this->device_revisions_[mac]", MESH_CPP)
        self.assertIn("this->device_revisions_.find(device->address_uint64())", MESH_CPP)


if __name__ == "__main__":
    unittest.main()
