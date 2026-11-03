#pragma once

#ifdef USE_ESP32
#include <cstring>
#include <bitset>
#include <deque>
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/log.h"
#include "esphome/components/esp32_ble_client/ble_client_base.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/mqtt/mqtt_client.h"
#include "device_info.h"
#include "device.h"

namespace esphome {
namespace awox_mesh {

using namespace esphome::mqtt;

/** UUID for Bluetooth GATT information service */
static std::string uuid_info_service = "00010203-0405-0607-0809-0a0b0c0d1910";
/** UUID for Bluetooth GATT notification characteristic */
static std::string uuid_notification_char = "00010203-0405-0607-0809-0a0b0c0d1911";
/** UUID for Bluetooth GATT command characteristic */
static std::string uuid_command_char = "00010203-0405-0607-0809-0a0b0c0d1912";
/** UUID for Bluetooth GATT pairing characteristic */
static std::string uuid_pair_char = "00010203-0405-0607-0809-0a0b0c0d1914";

#define COMMAND_ONLINE_STATUS_REPORT 0xDC
#define COMMAND_STATUS_REPORT 0xDB
#define COMMAND_ADDRESS_REPORT 0xD8
#define COMMAND_GROUP_ID_REPORT 0xD4

#define C_REQUEST_STATUS 0xda
#define C_POWER 0xd0
#define C_COLOR 0xe2
#define C_SEQUENCE 0xc8
#define C_CANDLE_MODE 0xc9
#define C_COLOR_BRIGHTNESS 0xf2
#define C_WHITE_BRIGHTNESS 0xf1
#define C_WHITE_TEMPERATURE 0xf0
#define C_SEQUENCE_COLOR_DURATION 0xf5
#define C_SEQUENCE_FADE_DURATION 0xf6
#define COMMAND_ADDRESS 0xE0
#define COMMAND_ADDRESS_REPORT_QUERY 0xE1
#define COMMAND_DEVICE_INFO_QUERY 0xEA
#define COMMAND_DEVICE_INFO_REPORT 0xEB
#define COMMAND_GROUP_ID_QUERY 0xDD

struct QueuedCommand {
  int command;
  std::string data;
  int dest;
  uint8_t attempts{0};
};

struct FoundDevice;
class AwoxMesh;

class MeshConnection : public esp32_ble_client::BLEClientBase {
  /**
   * Packet counter used to tag transmitted packets.
   */
  int packet_count = 1;
  uint32_t last_send_command = 0;
  uint32_t command_debounce_time = 180;

  std::deque<QueuedCommand> command_queue{};
  static constexpr size_t MAX_QUEUED_COMMANDS = 32;
  static constexpr uint8_t MAX_COMMAND_ATTEMPTS = 3;

  std::function<void()> disconnect_callback;

  std::string random_key;
  std::string session_key;

  std::string reverse_address;

  FoundDevice *found_device{nullptr};

  // Per-slot diagnostics, kept across reconnects until the next reboot.
  uint32_t connection_attempts_{0};
  uint32_t connection_failures_{0};
  uint32_t disconnect_count_{0};
  int last_disconnect_reason_{-1};
  int last_rssi_dbm_{-127};
  bool established_session_{false};
  uint32_t failed_command_writes_{0};
  uint32_t dropped_commands_{0};
  uint32_t last_notification_ms_{0};
  bool has_notification_{false};

  // Optional standard Device Information Service (0x180A) revisions.
  // A missing characteristic must never interrupt normal AwoX mesh operation.
  esp32_ble_client::BLECharacteristic *firmware_revision_char_{nullptr};
  esp32_ble_client::BLECharacteristic *hardware_revision_char_{nullptr};
  bool mesh_authenticated_{false};
  uint8_t next_revision_kind_{0};  // 0: firmware (0x2A26), 1: hardware (0x2A27)
  uint16_t pending_revision_handle_{0};
  uint8_t pending_revision_kind_{0};
  uint32_t pending_revision_started_ms_{0};
  void request_next_revision_();

  esp32_ble_client::BLECharacteristic *notification_char{nullptr};
  esp32_ble_client::BLECharacteristic *command_char{nullptr};
  esp32_ble_client::BLECharacteristic *pair_char{nullptr};

  void setup_connection();

  std::string combine_name_and_password() const;

  void generate_session_key(const std::string &data1, const std::string &data2);

  std::string key_encrypt(std::string &key) const;

  std::string encrypt_packet(std::string &packet) const;

  std::string decrypt_packet(std::string &packet) const;

  std::string build_packet(int dest, int command, const std::string &data);

  void handle_packet(std::string &packet);

  void queue_command(int command, const std::string &data, int dest = 0);

  void add_mesh_id(int mesh_id);
  void remove_mesh_id(int mesh_id);
  void clear_linked_mesh_ids();

  virtual void set_state(esp32_ble_tracker::ClientState st) override {
    esp32_ble_client::BLEClientBase::set_state(st);
    ESP_LOGI("awox.connection", "[%d] set_state %s", this->connection_index_, esp32_ble_tracker::client_state_to_string(st));
  }

 public:
  void loop() override;

  bool gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  void set_address(uint64_t address);

  void set_disconnect_callback(std::function<void()> &&f);

  bool write_command(int command, const std::string &data, int dest = 0, bool withResponse = false);

  void request_status();

  void set_power(int dest, bool state);

  void set_color(int dest, int red, int green, int blue);

  void set_color_brightness(int dest, int brightness);

  void set_white_brightness(int dest, int brightness);

  void set_white_temperature(int dest, int temp);

  void set_sequence(int dest, int sequence);

  void set_candle_mode(int dest);

  void set_sequence_fade_duration(int dest, int duration);

  void set_sequence_color_duration(int dest, int duration);

  void request_status_update(int dest);

  void request_device_info(Device *device);

  void request_group_info(Device *device);

  bool request_device_version(int dest);

  void connect_to(FoundDevice *found_device);

  int mesh_id();

  uint32_t connection_attempts() const { return this->connection_attempts_; }
  uint32_t connection_failures() const { return this->connection_failures_; }
  uint32_t disconnect_count() const { return this->disconnect_count_; }
  int last_disconnect_reason() const { return this->last_disconnect_reason_; }
  int last_rssi_dbm() const;
  size_t queued_commands() const { return this->command_queue.size(); }
  uint32_t failed_command_writes() const { return this->failed_command_writes_; }
  uint32_t dropped_commands() const { return this->dropped_commands_; }
  bool has_notification() const { return this->has_notification_; }
  uint32_t ms_since_notification(uint32_t now) const { return now - this->last_notification_ms_; }

  bool mesh_id_linked(int mesh_id);

  const std::vector<int> &get_linked_mesh_ids() const { return this->linked_mesh_ids_; }

 protected:
  friend class AwoxMesh;

  std::vector<int> linked_mesh_ids_;

  std::string mesh_name = "";
  std::string mesh_password = "";
  AwoxMesh *mesh_;
};

}  // namespace awox_mesh
}  // namespace esphome

#endif  // USE_ESP32
