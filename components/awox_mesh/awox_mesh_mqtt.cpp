#ifdef USE_ESP32

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <math.h>
#include <regex>

#include "esphome/components/mqtt/mqtt_const.h"
#include "esphome/components/mqtt/mqtt_component.h"
#include "esphome/core/application.h"
#include "esphome/core/log.h"

#include "awox_mesh_mqtt.h"
#include "awox_mesh.h"

namespace esphome {
namespace awox_mesh {

static const char *const TAG = "awox.mesh.mqtt";

static int convert_value_to_available_range(int value, int min_from, int max_from, int min_to, int max_to) {
  float normalized = (float) (value - min_from) / (float) (max_from - min_from);
  int new_value = std::min((int) round((normalized * (float) (max_to - min_to)) + min_to), max_to);

  return std::max(new_value, min_to);
}

static std::string get_product_code_as_hex_string(int product_id) {
  char value[15];
  sprintf(value, "Product: 0x%02X", product_id);
  return std::string((char *) value, 15);
}

void AwoxMeshMqtt::setup() {
  // Use retained MQTT messages to publish a default offline status for all devices
  global_mqtt_client->subscribe(
      global_mqtt_client->get_topic_prefix() + "/#", [this](const std::string &topic, const std::string &payload) {
        if (std::regex_match(topic, std::regex(global_mqtt_client->get_topic_prefix() + "/[0-9]+/availability"))) {
          ESP_LOGD(TAG, "Received topic: %s, %s", topic.c_str(), payload.c_str());
          if (payload == "online") {
            global_mqtt_client->publish(topic.c_str(), "offline");
          }
        }
      });
}

std::string AwoxMeshMqtt::get_discovery_topic_(const MQTTDiscoveryInfo &discovery_info, Device *device) const {
  return discovery_info.prefix + "/" + device->device_info->get_component_type() + "/awox-" +
         device->address_str_hex_only() + "/config";
}

std::string AwoxMeshMqtt::get_mqtt_topic_for_(MeshDestination *mesh_destination, const std::string &suffix) const {
  return global_mqtt_client->get_topic_prefix() + "/" + std::to_string(mesh_destination->dest()) + "/" + suffix;
}

void AwoxMeshMqtt::publish_connected(int active_connections, int online_devices,
                                     const std::vector<MeshConnection *> &connections) {
  if (!global_mqtt_client->is_connected()) {
    this->mqtt_was_connected_ = false;
    return;
  }
  const bool reconnected = !this->mqtt_was_connected_;
  if (reconnected) {
    this->mqtt_was_connected_ = true;
    this->last_published_state_.clear();
    this->last_published_availability_.clear();
    this->last_published_active_connections_ = -1;
    this->last_published_online_devices_ = -1;
    this->last_diagnostic_publish_ms_ = 0;
  }
  if (!this->published_connected) {
    // todo.... find proper solution
    // stop_listening_for_availability
    global_mqtt_client->unsubscribe(global_mqtt_client->get_topic_prefix() + "/#");
    this->published_connected = true;
  }

  if (this->last_published_active_connections_ < 0 ||
      (this->last_published_active_connections_ > 0) != (active_connections > 0)) {
    const std::string message = active_connections > 0 ? "online" : "offline";
    ESP_LOGI(TAG, "Publish mesh connection status: %s", message.c_str());

    global_mqtt_client->publish(global_mqtt_client->get_topic_prefix() + "/connected", message, 0, true);
  }

  const uint32_t now = esphome::millis();
  const bool changed = this->last_published_active_connections_ != active_connections ||
                       this->last_published_online_devices_ != online_devices;
  // Publish quickly for connection changes, but cap periodic telemetry at 30s.
  if (!changed && !reconnected && now - this->last_diagnostic_publish_ms_ < 30000) return;

  ESP_LOGI(TAG, "Publish connection info, %d connections, %d device online", active_connections, online_devices);

  const bool status_published = global_mqtt_client->publish_json(
      global_mqtt_client->get_topic_prefix() + "/connection_status",
      [&](JsonObject root) {
        root["active_connections"] = active_connections;
        root["online_devices"] = online_devices;

        for (int i = 0; i < connections.size(); i++) {
          JsonObject connection = root["connection_" + std::to_string(i)].to<JsonObject>();
          connection["connected"] = connections[i]->connected();
          connection["mac"] = connections[i]->connected() ? connections[i]->address_str() : "";
          connection["mesh_id"] = connections[i]->connected() ? std::to_string(connections[i]->mesh_id()) : "";
          connection["devices"] = connections[i]->get_linked_mesh_ids().size();

          // Lightweight per-slot diagnostics, serialized in the existing
          // connection_status topic rather than adding separate state topics.
          connection["connection_attempts"] = connections[i]->connection_attempts();
          connection["connection_failures"] = connections[i]->connection_failures();
          connection["disconnect_count"] = connections[i]->disconnect_count();
          connection["queue_length"] = connections[i]->queued_commands();
          connection["failed_command_writes"] = connections[i]->failed_command_writes();
          connection["dropped_commands"] = connections[i]->dropped_commands();
          const int rssi = connections[i]->last_rssi_dbm();
          if (rssi >= -127 && rssi <= 0) connection["rssi"] = rssi;
          else connection["rssi"] = nullptr;
          if (connections[i]->last_disconnect_reason() >= 0) {
            char reason[8];
            snprintf(reason, sizeof(reason), "0x%02X", connections[i]->last_disconnect_reason());
            connection["last_disconnect_reason"] = reason;
          } else {
            connection["last_disconnect_reason"] = nullptr;
          }
          if (connections[i]->has_notification()) {
            connection["last_notification_age_s"] = connections[i]->ms_since_notification(now) / 1000U;
          } else {
            connection["last_notification_age_s"] = nullptr;
          }

          std::stringstream mesh_ids;
          std::copy(connections[i]->get_linked_mesh_ids().begin(), connections[i]->get_linked_mesh_ids().end(),
                    std::ostream_iterator<int>(mesh_ids, ", "));
          connection["mesh_ids"] = mesh_ids.str().substr(0, mesh_ids.str().size() - 2);
        }
      },
      0, false);
  if (status_published) {
    this->last_published_online_devices_ = online_devices;
    this->last_published_active_connections_ = active_connections;
    this->last_diagnostic_publish_ms_ = now;
  }
  if (reconnected) {
    this->publish_connection_sensor_discovery(connections);
    this->mesh_->refresh_mqtt_discovery();
  }
}

void AwoxMeshMqtt::publish_availability(Device *device) {
  if (this->last_published_availability_.count(device->dest()) &&
      this->last_published_availability_[device->dest()] == device->online) {
    return;
  }
  const std::string message = device->online ? "online" : "offline";
  ESP_LOGI(TAG, "Publish online/offline for device %u - %s", device->mesh_id, message.c_str());
  if (global_mqtt_client->publish(this->get_mqtt_topic_for_(device, "availability"), message, 0, true))
    this->last_published_availability_[device->dest()] = device->online;
}

void AwoxMeshMqtt::publish_availability(Group *group) {
  if (this->last_published_availability_.count(group->dest()) &&
      this->last_published_availability_[group->dest()] == group->online) {
    return;
  }
  const std::string message = group->online ? "online" : "offline";
  ESP_LOGI(TAG, "Publish online/offline for group %u - %s", group->group_id, message.c_str());
  if (global_mqtt_client->publish(this->get_mqtt_topic_for_(group, "availability"), message, 0, true))
    this->last_published_availability_[group->dest()] = group->online;
}

void AwoxMeshMqtt::publish_state(MeshDestination *mesh_destination) {
  if (!mesh_destination->can_publish_state()) {
    ESP_LOGW(TAG, "[%u] Can not yet send publish state for %s", mesh_destination->dest(), mesh_destination->type());
    return;
  }

  if (this->last_published_state_.count(mesh_destination->dest()) &&
      memcmp(this->last_published_state_[mesh_destination->dest()].state, mesh_destination->state_as_char().state, 7) ==
          0) {
    ESP_LOGV(TAG, "[%u] No need to update state is equal to last publication for %s", mesh_destination->dest(),
             mesh_destination->type());
    return;
  }

  ESP_LOGD(TAG, "Publish state for %s", mesh_destination->state_as_string().c_str());

  bool published = false;
  if (mesh_destination->device_info->has_feature(FEATURE_LIGHT_MODE)) {
    published = global_mqtt_client->publish_json(
        this->get_mqtt_topic_for_(mesh_destination, "state"),
        [this, mesh_destination](JsonObject root) {
          root["state"] = mesh_destination->state ? "ON" : "OFF";

          if (mesh_destination->color_mode) {
            root["color_mode"] = "rgb";
            root["brightness"] =
                convert_value_to_available_range(mesh_destination->color_brightness, 0xa, 0x64, 0, 255);
          } else {
            if (mesh_destination->device_info->has_feature(FEATURE_WHITE_TEMPERATURE)) {
              root["color_mode"] = "color_temp";
              root["color_temp"] = convert_value_to_available_range(mesh_destination->temperature, 0, 0x7f, 153, 370);
            } else {
              root["color_mode"] = "brightness";
            }
            root["brightness"] = convert_value_to_available_range(mesh_destination->white_brightness, 1, 0x7f, 0, 255);
          }

          if (mesh_destination->candle_mode) {
            root["effect"] = "candle";
          } else if (mesh_destination->sequence_mode) {
            root["effect"] = "color loop";
          }

          // https://developers.home-assistant.io/docs/core/entity/light#color-mode-when-rendering-effects
          if (mesh_destination->candle_mode || mesh_destination->sequence_mode) {
            root["color_mode"] = "brightness";
          }

          JsonObject color = root["color"].to<JsonObject>();
          color["r"] = mesh_destination->R;
          color["g"] = mesh_destination->G;
          color["b"] = mesh_destination->B;
        },
        0, true);
  } else {
    published = global_mqtt_client->publish(this->get_mqtt_topic_for_(mesh_destination, "state"),
                                mesh_destination->state ? "ON" : "OFF", mesh_destination->state ? 2 : 3, 0, true);
  }
  if (published) this->last_published_state_[mesh_destination->dest()] = mesh_destination->state_as_char();
}

// All diagnostic entities reuse the single connection_status payload. Discovery
// is retained according to the normal ESPHome MQTT discovery configuration.
static void publish_diagnostic_sensor(const MQTTDiscoveryInfo &discovery, const std::string &slug,
                                      const std::string &name, const std::string &template_value,
                                      const std::string &icon, const char *unit = nullptr,
                                      const char *state_class = nullptr, bool enabled = false) {
  const std::string node = str_sanitize(App.get_name());
  global_mqtt_client->publish_json(
      discovery.prefix + "/sensor/" + node + "/" + slug + "/config",
      [=](JsonObject root) {
        root[MQTT_NAME] = name;
        root[MQTT_UNIQUE_ID] = "awox-" + get_mac_address() + "-" + slug;
        root[MQTT_ENTITY_CATEGORY] = "diagnostic";
        root[MQTT_ICON] = icon;
        root[MQTT_ENABLED_BY_DEFAULT] = enabled;
        root[MQTT_STATE_TOPIC] = global_mqtt_client->get_topic_prefix() + "/connection_status";
        root[MQTT_AVAILABILITY_TOPIC] = global_mqtt_client->get_topic_prefix() + "/status";
        root[MQTT_VALUE_TEMPLATE] = template_value;
        if (unit != nullptr) root["unit_of_measurement"] = unit;
        if (state_class != nullptr) root["state_class"] = state_class;
        JsonObject device = root[MQTT_DEVICE].to<JsonObject>();
        device[MQTT_DEVICE_IDENTIFIERS] = get_mac_address();
      },
      0, discovery.retain);
}

void AwoxMeshMqtt::publish_connection_sensor_discovery(const std::vector<MeshConnection *> &connections) {
  const MQTTDiscoveryInfo &discovery_info = global_mqtt_client->get_discovery_info();
  const std::string sanitized_name = str_sanitize(App.get_name());

  publish_diagnostic_sensor(discovery_info, "active-connections", "AwoX active BLE connections",
                            "{{ value_json.active_connections }}", "mdi:connection", nullptr, "measurement", true);
  publish_diagnostic_sensor(discovery_info, "online-devices", "AwoX online mesh devices",
                            "{{ value_json.online_devices }}", "mdi:lightbulb-multiple-outline",
                            nullptr, "measurement", true);

  for (int i = 0; i < connections.size(); i++) {
    const std::string slot = "connection-" + std::to_string(i);
    const std::string field = "value_json.connection_" + std::to_string(i);
    publish_diagnostic_sensor(discovery_info, slot + "-rssi", "Connection " + std::to_string(i) + " RSSI",
                              "{{ " + field + ".rssi }}", "mdi:signal", "dBm", "measurement", true);
    publish_diagnostic_sensor(discovery_info, slot + "-attempts",
                              "Connection " + std::to_string(i) + " attempts",
                              "{{ " + field + ".connection_attempts }}", "mdi:counter",
                              nullptr, "total_increasing", true);
    publish_diagnostic_sensor(discovery_info, slot + "-failures",
                              "Connection " + std::to_string(i) + " failed attempts",
                              "{{ " + field + ".connection_failures }}", "mdi:alert-circle-outline",
                              nullptr, "total_increasing");
    publish_diagnostic_sensor(discovery_info, slot + "-disconnects",
                              "Connection " + std::to_string(i) + " disconnects",
                              "{{ " + field + ".disconnect_count }}", "mdi:link-off",
                              nullptr, "total_increasing", true);
    publish_diagnostic_sensor(discovery_info, slot + "-reason",
                              "Connection " + std::to_string(i) + " last disconnect reason",
                              "{{ " + field + ".last_disconnect_reason }}", "mdi:information-outline",
                              nullptr, nullptr, true);
    publish_diagnostic_sensor(discovery_info, slot + "-notification-age",
                              "Connection " + std::to_string(i) + " last notification age",
                              "{{ " + field + ".last_notification_age_s }}", "mdi:timer-outline",
                              "s", "measurement", true);
    publish_diagnostic_sensor(discovery_info, slot + "-queue",
                              "Connection " + std::to_string(i) + " queued commands",
                              "{{ " + field + ".queue_length }}", "mdi:format-list-numbered",
                              nullptr, "measurement");
    publish_diagnostic_sensor(discovery_info, slot + "-write-errors",
                              "Connection " + std::to_string(i) + " failed command writes",
                              "{{ " + field + ".failed_command_writes }}", "mdi:alert-circle-outline",
                              nullptr, "total_increasing");
    publish_diagnostic_sensor(discovery_info, slot + "-dropped",
                              "Connection " + std::to_string(i) + " dropped commands",
                              "{{ " + field + ".dropped_commands }}", "mdi:delete-alert-outline",
                              nullptr, "total_increasing");
    global_mqtt_client->publish_json(
        discovery_info.prefix + "/sensor/" + sanitized_name + "/connection-" + std::to_string(i) + "-devices/config",
        [this, i, discovery_info](JsonObject root) {
          // Entity
          root[MQTT_NAME] = "Connection " + std::to_string(i) + " devices";
          root[MQTT_UNIQUE_ID] = "awox-connection-" + std::to_string(i) + "-devices";
          root[MQTT_ENTITY_CATEGORY] = "diagnostic";
          root[MQTT_ICON] = "mdi:counter";
          root[MQTT_ENABLED_BY_DEFAULT] = false;

          // State and command topic
          root[MQTT_STATE_TOPIC] = global_mqtt_client->get_topic_prefix() + "/connection_status";

          root[MQTT_AVAILABILITY_TOPIC] = global_mqtt_client->get_topic_prefix() + "/status";
          root[MQTT_VALUE_TEMPLATE] = "{{ value_json.connection_" + std::to_string(i) + ".devices }}";

          // Device
          JsonObject device_info = root[MQTT_DEVICE].to<JsonObject>();
          device_info[MQTT_DEVICE_IDENTIFIERS] = get_mac_address();
        },
        0, discovery_info.retain);

    global_mqtt_client->publish_json(
        discovery_info.prefix + "/sensor/" + sanitized_name + "/connection-" + std::to_string(i) + "-mesh-ids/config",
        [this, i, discovery_info](JsonObject root) {
          // Entity
          root[MQTT_NAME] = "Connection " + std::to_string(i) + " Mesh ID's";
          root[MQTT_UNIQUE_ID] = "awox-connection-" + std::to_string(i) + "-mesh-ids";
          root[MQTT_ENTITY_CATEGORY] = "diagnostic";
          root[MQTT_ICON] = "mdi:vector-polyline";
          root[MQTT_ENABLED_BY_DEFAULT] = false;

          // State and command topic
          root[MQTT_STATE_TOPIC] = global_mqtt_client->get_topic_prefix() + "/connection_status";

          root[MQTT_AVAILABILITY_TOPIC] = global_mqtt_client->get_topic_prefix() + "/status";
          root[MQTT_VALUE_TEMPLATE] = "{{ value_json.connection_" + std::to_string(i) + ".mesh_ids }}";

          // Device
          JsonObject device_info = root[MQTT_DEVICE].to<JsonObject>();
          device_info[MQTT_DEVICE_IDENTIFIERS] = get_mac_address();
        },
        0, discovery_info.retain);

    global_mqtt_client->publish_json(
        discovery_info.prefix + "/sensor/" + sanitized_name + "/connection-" + std::to_string(i) + "-mesh-id/config",
        [this, i, discovery_info](JsonObject root) {
          // Entity
          root[MQTT_NAME] = "Connection " + std::to_string(i) + " Mesh ID";
          root[MQTT_UNIQUE_ID] = "awox-connection-" + std::to_string(i) + "-mesh-id";
          root[MQTT_ENTITY_CATEGORY] = "diagnostic";
          root[MQTT_ICON] = "mdi:vector-point-select";
          root[MQTT_ENABLED_BY_DEFAULT] = false;

          // State and command topic
          root[MQTT_STATE_TOPIC] = global_mqtt_client->get_topic_prefix() + "/connection_status";

          root[MQTT_AVAILABILITY_TOPIC] = global_mqtt_client->get_topic_prefix() + "/status";
          root[MQTT_VALUE_TEMPLATE] = "{{ value_json.connection_" + std::to_string(i) + ".mesh_id }}";

          // Device
          JsonObject device_info = root[MQTT_DEVICE].to<JsonObject>();
          device_info[MQTT_DEVICE_IDENTIFIERS] = get_mac_address();
        },
        0, discovery_info.retain);

    global_mqtt_client->publish_json(
        discovery_info.prefix + "/sensor/" + sanitized_name + "/connection-" + std::to_string(i) + "-mac/config",
        [this, i, discovery_info](JsonObject root) {
          // Entity
          root[MQTT_NAME] = "Connection " + std::to_string(i) + " mac address";
          root[MQTT_UNIQUE_ID] = "awox-connection-" + std::to_string(i) + "-mac";
          root[MQTT_ENTITY_CATEGORY] = "diagnostic";
          root[MQTT_ICON] = "mdi:information";
          root[MQTT_ENABLED_BY_DEFAULT] = false;

          // State and command topic
          root[MQTT_STATE_TOPIC] = global_mqtt_client->get_topic_prefix() + "/connection_status";

          root[MQTT_AVAILABILITY_TOPIC] = global_mqtt_client->get_topic_prefix() + "/status";
          root[MQTT_VALUE_TEMPLATE] = "{{ value_json.connection_" + std::to_string(i) + ".mac }}";

          // Device
          JsonObject device_info = root[MQTT_DEVICE].to<JsonObject>();
          device_info[MQTT_DEVICE_IDENTIFIERS] = get_mac_address();
        },
        0, discovery_info.retain);

    global_mqtt_client->publish_json(
        discovery_info.prefix + "/binary_sensor/" + sanitized_name + "/connection-" + std::to_string(i) +
            "-connected/config",
        [this, i, discovery_info](JsonObject root) {
          // Entity
          root[MQTT_NAME] = "Connection " + std::to_string(i) + " connected";
          root[MQTT_UNIQUE_ID] = "awox-connection-" + std::to_string(i) + "-connected";
          root[MQTT_ENTITY_CATEGORY] = "diagnostic";
          root[MQTT_ICON] = "mdi:connection";

          root[MQTT_AVAILABILITY_TOPIC] = global_mqtt_client->get_topic_prefix() + "/status";

          // State and command topic
          root[MQTT_STATE_TOPIC] = global_mqtt_client->get_topic_prefix() + "/connection_status";

          root[MQTT_PAYLOAD_ON] = true;
          root[MQTT_PAYLOAD_OFF] = false;
          root[MQTT_VALUE_TEMPLATE] = "{{ value_json.connection_" + std::to_string(i) + ".connected }}";

          // Device
          JsonObject device_info = root[MQTT_DEVICE].to<JsonObject>();
          device_info[MQTT_DEVICE_IDENTIFIERS] = get_mac_address();
        },
        0, discovery_info.retain);
  }
}

bool AwoxMeshMqtt::send_discovery(Device *device) {
  if (!device->address_set() || device->device_info == nullptr) {
    ESP_LOGW(TAG, "'%s': Can not yet send discovery, mac address not known...",
             std::to_string(device->mesh_id).c_str());
    return false;
  }

  ESP_LOGD(TAG, "[%u]: Sending discovery productID: 0x%02X (%s - %s) mac: %s", device->mesh_id,
           device->device_info->get_product_id(), device->device_info->get_name(), device->device_info->get_model(),
           device->address_str().c_str());

  const MQTTDiscoveryInfo &discovery_info = global_mqtt_client->get_discovery_info();

  const bool published = global_mqtt_client->publish_json(
      this->get_discovery_topic_(discovery_info, device),
      [this, device, discovery_info](JsonObject root) {
        root["schema"] = "json";

        // Entity
        root[MQTT_NAME] = nullptr;
        root[MQTT_UNIQUE_ID] = "awox-" + device->address_str() + "-" + device->device_info->get_component_type();

        if (strlen(device->device_info->get_icon()) > 0) {
          root[MQTT_ICON] = device->device_info->get_icon();
        }

        // State and command topic
        root[MQTT_STATE_TOPIC] = this->get_mqtt_topic_for_(device, "state");
        root[MQTT_COMMAND_TOPIC] = this->get_mqtt_topic_for_(device, "command");

        // Availavility topics
        JsonArray availability = root[MQTT_AVAILABILITY].to<JsonArray>();
        auto availability_topic_1 = availability.add<JsonObject>();
        availability_topic_1[MQTT_TOPIC] = this->get_mqtt_topic_for_(device, "availability");
        auto availability_topic_2 = availability.add<JsonObject>();
        availability_topic_2[MQTT_TOPIC] = global_mqtt_client->get_topic_prefix() + "/status";
        auto availability_topic_3 = availability.add<JsonObject>();
        availability_topic_3[MQTT_TOPIC] = global_mqtt_client->get_topic_prefix() + "/connected";
        root[MQTT_AVAILABILITY_MODE] = "all";

        // Features
        if (device->device_info->has_feature(FEATURE_WHITE_BRIGHTNESS) ||
            device->device_info->has_feature(FEATURE_COLOR_BRIGHTNESS)) {
          root["brightness"] = true;
          root["brightness_scale"] = 255;
        }

        JsonArray color_modes = root["supported_color_modes"].to<JsonArray>();

        if (device->device_info->has_feature(FEATURE_COLOR)) {
          color_modes.add("rgb");

          root["effect"] = true;
          JsonArray effect_list = root[MQTT_EFFECT_LIST].to<JsonArray>();
          effect_list.add("candle");
          effect_list.add("color loop");
          effect_list.add("stop");
        }

        if (device->device_info->has_feature(FEATURE_WHITE_TEMPERATURE)) {
          color_modes.add("color_temp");

          root[MQTT_MIN_MIREDS] = 153;
          root[MQTT_MAX_MIREDS] = 370;
        }

        // brightness should always be used alone
        // https://developers.home-assistant.io/docs/core/entity/light/#color-modes
        if (color_modes.size() == 0 && device->device_info->has_feature(FEATURE_WHITE_BRIGHTNESS)) {
          color_modes.add("brightness");
        }

        if (color_modes.size() == 0) {
          color_modes.add("onoff");
        }

        // Device
        JsonObject device_info = root[MQTT_DEVICE].to<JsonObject>();

        JsonArray identifiers = device_info[MQTT_DEVICE_IDENTIFIERS].to<JsonArray>();
        identifiers.add("esp-awox-mesh-" + std::to_string(device->mesh_id));
        identifiers.add(device->address_str());

        device_info[MQTT_DEVICE_NAME] = device->device_info->get_name();

        if (strlen(device->device_info->get_model()) == 0) {
          device_info[MQTT_DEVICE_MODEL] = get_product_code_as_hex_string(device->device_info->get_product_id());
        } else {
          device_info[MQTT_DEVICE_MODEL] = std::string(device->device_info->get_model());
        }
        device_info[MQTT_DEVICE_MANUFACTURER] = device->device_info->get_manufacturer();
        device_info["via_device"] = get_mac_address();
        device_info["serial_number"] = "mesh-id " + std::to_string(device->mesh_id);
      },
      0, discovery_info.retain);

  if (!published) {
    return false;
  }

  const bool newly_subscribed = this->subscribed_destinations_.insert(device->dest()).second;
  if (newly_subscribed && device->device_info->has_feature(FEATURE_LIGHT_MODE)) {
    global_mqtt_client->subscribe_json(
        this->get_mqtt_topic_for_(device, "command"),
        [this, device](const std::string &topic, JsonObject root) { this->process_incomming_command(device, root); });
  } else if (newly_subscribed) {
    global_mqtt_client->subscribe(this->get_mqtt_topic_for_(device, "command"),
                                  [this, device](const std::string &topic, const std::string &payload) {
                                    ESP_LOGI(TAG, "command %s - %s", topic.c_str(), payload.c_str());
                                    auto val = parse_on_off(payload.c_str());
                                    switch (val) {
                                      case PARSE_ON:
                                        device->state = true;
                                        this->mesh_->set_power(device->mesh_id, true);
                                        break;
                                      case PARSE_OFF:
                                        device->state = false;
                                        this->mesh_->set_power(device->mesh_id, false);
                                        break;
                                      case PARSE_TOGGLE:
                                        device->state = !device->state;
                                        this->mesh_->set_power(device->mesh_id, device->state);
                                        break;
                                      case PARSE_NONE:
                                        break;
                                    }
                                  });
  }
  this->publish_availability(device);
  return true;
}

bool AwoxMeshMqtt::send_group_discovery(Group *group) {
  if (group->device_info == nullptr) {
    ESP_LOGW(TAG, "'%s': Can not yet send discovery, component_type not known...",
             std::to_string(group->group_id).c_str());
    return false;
  }

  // Ignore groups that don't represent lights (for example plug-only groups).
  if (!group->device_info->has_feature(FEATURE_LIGHT_MODE)) {
    ESP_LOGI(TAG, "[%u] Non-light group ignored", group->group_id);
    return true;
  }

  const MQTTDiscoveryInfo &discovery_info = global_mqtt_client->get_discovery_info();

  const bool published = global_mqtt_client->publish_json(
      discovery_info.prefix + "/" + group->device_info->get_component_type() + "/group-" +
          std::to_string(group->group_id) + "/config",
      [this, group, discovery_info](JsonObject root) {
        root["schema"] = "json";
        // Entity
        root[MQTT_NAME] = nullptr;
        root[MQTT_UNIQUE_ID] = "group-" + std::to_string(group->group_id);

        root[MQTT_ICON] = "mdi:lightbulb-group";

        // State and command topic
        root[MQTT_STATE_TOPIC] = this->get_mqtt_topic_for_(group, "state");
        root[MQTT_COMMAND_TOPIC] = this->get_mqtt_topic_for_(group, "command");

        // Availavility topics
        JsonArray availability = root[MQTT_AVAILABILITY].to<JsonArray>();
        auto availability_topic_1 = availability.add<JsonObject>();
        availability_topic_1[MQTT_TOPIC] = this->get_mqtt_topic_for_(group, "availability");
        auto availability_topic_2 = availability.add<JsonObject>();
        availability_topic_2[MQTT_TOPIC] = global_mqtt_client->get_topic_prefix() + "/status";
        auto availability_topic_3 = availability.add<JsonObject>();
        availability_topic_3[MQTT_TOPIC] = global_mqtt_client->get_topic_prefix() + "/connected";
        root[MQTT_AVAILABILITY_MODE] = "all";

        // Features
        if (group->device_info->has_feature(FEATURE_WHITE_BRIGHTNESS) ||
            group->device_info->has_feature(FEATURE_COLOR_BRIGHTNESS)) {
          root["brightness"] = true;
          root["brightness_scale"] = 255;
        }

        JsonArray color_modes = root["supported_color_modes"].to<JsonArray>();

        if (group->device_info->has_feature(FEATURE_COLOR)) {
          color_modes.add("rgb");

          root["effect"] = true;
          JsonArray effect_list = root[MQTT_EFFECT_LIST].to<JsonArray>();
          effect_list.add("candle");
          effect_list.add("color loop");
          effect_list.add("stop");
        }

        if (group->device_info->has_feature(FEATURE_WHITE_TEMPERATURE)) {
          color_modes.add("color_temp");

          root[MQTT_MIN_MIREDS] = 153;
          root[MQTT_MAX_MIREDS] = 370;
        }

        // brightness should always be used alone
        // https://developers.home-assistant.io/docs/core/entity/light/#color-modes
        if (color_modes.size() == 0 && group->device_info->has_feature(FEATURE_WHITE_BRIGHTNESS)) {
          color_modes.add("brightness");
        }

        if (color_modes.size() == 0) {
          color_modes.add("onoff");
        }

        // Device
        JsonObject device_info = root[MQTT_DEVICE].to<JsonObject>();

        JsonArray identifiers = device_info[MQTT_DEVICE_IDENTIFIERS].to<JsonArray>();
        identifiers.add("esp-awox-mesh-group-" + std::to_string(group->group_id));

        device_info[MQTT_DEVICE_MODEL] = "Group - " + std::to_string(group->group_id);
        device_info[MQTT_DEVICE_MANUFACTURER] = "ESPHome AwoX BLE mesh";

        device_info[MQTT_DEVICE_NAME] = "Group " + std::to_string(group->group_id);

        device_info["via_device"] = get_mac_address();
        device_info["serial_number"] = "group-id " + std::to_string(group->group_id);
      },
      0, discovery_info.retain);

  if (!published) return false;

  if (this->subscribed_destinations_.insert(group->dest()).second) {
    global_mqtt_client->subscribe_json(
        this->get_mqtt_topic_for_(group, "command"),
        [this, group](const std::string &topic, JsonObject root) { this->process_incomming_command(group, root); });
  }
  this->publish_availability(group);
  return true;
}

void AwoxMeshMqtt::process_incomming_command(MeshDestination *mesh_destination, JsonObject root) {
  int dest = mesh_destination->dest();
  bool state_set = false;

  ESP_LOGI(TAG, "[%u] Process command %s", dest, mesh_destination->type());

  if (root["fade_duration"].is<JsonVariant>()) {
    ESP_LOGD(TAG, "[%u] set sequence fade_duration %d", dest, (int) root["fade_duration"]);
    this->mesh_->set_sequence_fade_duration(dest, (int) root["fade_duration"]);
  }

  if (root["color_duration"].is<JsonVariant>()) {
    ESP_LOGD(TAG, "[%u] set sequence color_duration %d", dest, (int) root["color_duration"]);
    this->mesh_->set_sequence_color_duration(dest, (int) root["color_duration"]);
  }

  if (root["color"].is<JsonVariant>()) {
    JsonObject color = root["color"];

    state_set = true;
    mesh_destination->state = true;
    mesh_destination->color_mode = true;
    mesh_destination->R = (int) color["r"];
    mesh_destination->G = (int) color["g"];
    mesh_destination->B = (int) color["b"];

    ESP_LOGD(TAG, "[%u] Process command color %d %d %d", dest, (int) color["r"], (int) color["g"], (int) color["b"]);

    this->mesh_->set_color(dest, (int) color["r"], (int) color["g"], (int) color["b"]);
  }

  if (root["brightness"].is<JsonVariant>() && !root["color_temp"].is<JsonVariant>() &&
      (root["color"].is<JsonVariant>() || mesh_destination->color_mode)) {
    int brightness = convert_value_to_available_range((int) root["brightness"], 0, 255, 0xa, 0x64);

    state_set = true;
    mesh_destination->state = true;
    mesh_destination->color_brightness = brightness;

    ESP_LOGD(TAG, "[%u] Process command color_brightness %d", dest, (int) root["brightness"]);
    this->mesh_->set_color_brightness(dest, brightness);

  } else if (root["brightness"].is<JsonVariant>()) {
    int brightness = convert_value_to_available_range((int) root["brightness"], 0, 255, 1, 0x7f);

    state_set = true;
    mesh_destination->state = true;
    mesh_destination->white_brightness = brightness;

    ESP_LOGD(TAG, "[%u] Process command white_brightness %d", dest, (int) root["brightness"]);
    this->mesh_->set_white_brightness(dest, brightness);
  }

  if (root["color_temp"].is<JsonVariant>()) {
    int temperature = convert_value_to_available_range((int) root["color_temp"], 153, 370, 0, 0x7f);

    state_set = true;
    mesh_destination->state = true;
    mesh_destination->color_mode = false;
    mesh_destination->temperature = temperature;

    ESP_LOGD(TAG, "[%u] Process command color_temp %d", dest, (int) root["color_temp"]);
    this->mesh_->set_white_temperature(dest, temperature);
  }

  if (root["effect"].is<JsonVariant>()) {
    state_set = true;
    mesh_destination->state = true;
    mesh_destination->sequence_mode = false;
    mesh_destination->candle_mode = false;
    if (root["effect"] == "color loop") {
      mesh_destination->sequence_mode = true;
      this->mesh_->set_sequence(dest, 0);
    } else if (root["effect"] == "candle") {
      mesh_destination->candle_mode = true;
      this->mesh_->set_candle_mode(dest);
    } else {
      if (mesh_destination->color_mode) {
        this->mesh_->set_color(dest, mesh_destination->R, mesh_destination->G, mesh_destination->B);
      } else {
        this->mesh_->set_white_temperature(dest, mesh_destination->temperature);
      }
    }
  }

  if (root["state"].is<JsonVariant>()) {
    ESP_LOGD(TAG, "[%u] Process command state", dest);
    auto val = parse_on_off(root["state"]);
    switch (val) {
      case PARSE_ON:
        mesh_destination->state = true;
        if (!state_set) {
          this->mesh_->set_power(dest, true);
        }
        break;
      case PARSE_OFF:
        mesh_destination->state = false;
        this->mesh_->set_power(dest, false);
        break;
      case PARSE_TOGGLE:
        mesh_destination->state = !mesh_destination->state;
        this->mesh_->set_power(dest, mesh_destination->state);
        break;
      case PARSE_NONE:
        break;
    }
  }

  this->publish_state(mesh_destination);
}

}  // namespace awox_mesh
}  // namespace esphome

#endif
