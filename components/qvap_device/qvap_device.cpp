
#include "qvap_device.h"

namespace esphome {
namespace qvap_device {

void QVAPDevice::set_ibeacon_name(const std::string &name) {
  this->ibeacon_name = name;
}

void QVAPDevice::set_ibeacon_address(const std::string &address) {
  this->ibeacon_address = address;
}

void QVAPDevice::setup() {
  ESP_LOGI(TAG, "Setting up QVAP Device...");
  this->connect_to_ble();
}

void QVAPDevice::connect_to_ble() {
  this->parent()->add_on_connect_callback([this]() {
    ESP_LOGI(TAG, "Connected to QVapDevice, initializing...");
    this->state = STATE_INIT_CONNECTION;
  });

  this->parent()->add_on_disconnect_callback([this]() {
    ESP_LOGI(TAG, "Disconnected from QVapDevice, reconnecting...");
    this->connect_to_ble();
  });

  this->parent()->add_on_notify_callback([this](const std::vector<uint8_t> &data) {
    this->on_data(data.data(), data.size());
  });
}

void QVAPDevice::loop() {
  switch (this->state) {
    case STATE_INIT_CONNECTION:
      this->init_connection();
      this->state = STATE_SETUP_INITIAL_PARAMS;
      break;
    case STATE_SETUP_INITIAL_PARAMS:
      this->setup_initial_params();
      this->state = STATE_PREPARE_DEVICE;
      break;
    case STATE_PREPARE_DEVICE:
      this->prepare_device();
      this->state = STATE_ADDITIONAL_SETUP;
      break;
    case STATE_ADDITIONAL_SETUP:
      this->additional_setup();
      this->state = STATE_FINAL_SETUP;
      break;
    case STATE_FINAL_SETUP:
      this->final_setup();
      this->state = STATE_PERIODIC_UPDATE;
      break;
    case STATE_PERIODIC_UPDATE:
      this->start_periodic_updates();
      break;
    case STATE_IDLE:
    default:
      break;
  }
}

void QVAPDevice::init_connection() {
  this->send_command(INIT_CONNECTION);
}

void QVAPDevice::setup_initial_params() {
  this->send_command(SETUP_INITIAL_PARAMS);
}

void QVAPDevice::prepare_device() {
  this->send_command(PREPARE_DEVICE);
}

void QVAPDevice::additional_setup() {
  this->send_command(ADDITIONAL_SETUP);
}

void QVAPDevice::final_setup() {
  this->send_command(FINAL_SETUP);
}

void QVAPDevice::final_stage() {
  std::vector<uint8_t> buffer(20, 0);
  buffer[1] = 6;
  if (this->application_firmware & 1) {  // Check if firmware is valid
    ESP_LOGI(TAG, "Firmware application valid. Starting periodic updates.");
    buffer[0] = BOOTLOADER_UPDATE;
  } else {
    buffer[0] = SETUP_INITIAL_PARAMS;
  }
  this->parent()->write_value(SERVICE_UUID_QVAP, CHARACTERISTIC_UUID, buffer.data(), buffer.size());
}

void QVAPDevice::send_command(Command command, const std::vector<uint8_t> &data) {
  uint8_t buffer[20] = {0};
  buffer[0] = command;
  std::copy(data.begin(), data.end(), buffer + 1);
  this->parent()->write_value(SERVICE_UUID_QVAP, CHARACTERISTIC_UUID, buffer, sizeof(buffer));
}

void QVAPDevice::start_periodic_updates() {
  this->send_command(PREPARE_DEVICE);
  this->send_command(SETUP_INITIAL_PARAMS);
}

void QVAPDevice::on_data(const uint8_t *data, size_t length) {
  ESP_LOGI(TAG, "Notification received: %s", format_hex_pretty(data, length).c_str());
  this->parse_response(data, length);
}

void QVAPDevice::parse_response(const uint8_t *data, size_t length) {
  uint8_t command = data[0];
  if (command == 2) {  // Command that updates the application firmware status
    this->application_firmware = data[1];
    std::string firmware_version(reinterpret_cast<const char*>(data + 2), 6);
    std::string bootloader_version(reinterpret_cast<const char*>(data + 11), 6);
    ESP_LOGI(TAG, "Application Firmware: %s, Bootloader: %s", firmware_version.c_str(), bootloader_version.c_str());

    // Handle further firmware version comparison and update
  } else if (command == 1 || command == 48) {
    if (this->application_firmware & 1 && command != 48) {
      this->current_temp = (data[3] << 8) + data[2];
      this->set_temp = (data[5] << 8) + data[4];
      this->boost_temp = data[6];
      this->superboost_temp = data[7];
      this->battery_level = data[8];
      this->auto_shutoff = (data[10] << 8) + data[9];
      this->heater_mode = data[11];
      this->charger_status = data[13];
      this->settings_flags = data[14];

      ESP_LOGI(TAG, "Current Temp: %d, Set Temp: %d, Boost Temp: %d, Superboost Temp: %d, Battery: %d, Auto-shutoff: %d, Heater Mode: %d, Charger: %d, Settings: %d",
               this->current_temp, this->set_temp, this->boost_temp, this->superboost_temp, this->battery_level, this->auto_shutoff, this->heater_mode, this->charger_status, this->settings_flags);

      // Publish sensor values
      //if (this->current_temp_sensor != nullptr)
      //  this->current_temp_sensor->publish_state(this->current_temp);
      //if (this->set_temp_sensor != nullptr)
      //  this->set_temp_sensor->publish_state(this->set_temp);
      //if (this->boost_temp_sensor != nullptr)
      //  this->boost_temp_sensor->publish_state(this->boost_temp);
      //if (this->superboost_temp_sensor != nullptr)
      //  this->superboost_temp_sensor->publish_state(this->superboost_temp);
      //if (this->battery_level_sensor != nullptr)
      //  this->battery_level_sensor->publish_state(this->battery_level);
      //if (this->auto_shutoff_sensor != nullptr)
      //  this->auto_shutoff_sensor->publish_state(this->auto_shutoff);
      //if (this->heater_mode_sensor != nullptr)
      //  this->heater_mode_sensor->publish_state(this->heater_mode);
      //if (this->charger_status_sensor != nullptr)
      //  this->charger_status_sensor->publish_state(this->charger_status);
      //if (this->settings_flags_sensor != nullptr)
      //  this->settings_flags_sensor->publish_state(this->settings_flags);
    } else {
      ESP_LOGI(TAG, "_______cmd0x01 - bootloader");
      // Handle bootloader update steps
      uint8_t status = data[1];
      if (status == 1) {
        this->data_idx_qvap += 1;
        if (this->data_idx_qvap >= this->page_size_qvap / this->data_size_firmware_per_packet) {
          ESP_LOGI(TAG, "page write start request");
        } else {
          ESP_LOGI(TAG, "page data write request");
        }
        this->write_page_data_sequence_qvap();
      } else if (status == 2) {
        this->page_idx_qvap += 1;
        this->data_idx_qvap = 0;
        ESP_LOGI(TAG, "page %d write done", this->page_idx_qvap);
        if (this->page_idx_qvap < this->binary_number_of_pages) {
          this->write_page_data_sequence_qvap();
        } else {
          delay(200);
          this->start_qvap_application(true);
        }
      } else if (status == 3) {
        // Additional status handling if needed
      } else if (status == 4) {
        // Additional status handling if needed
      } else if (status == 5) {
        // Additional status handling if needed
      } else if (status == 6) {
        uint8_t connection_interval = data[2];
        ESP_LOGI(TAG, "connection interval %d", connection_interval);
        if (connection_interval < 25) {
          ESP_LOGI(TAG, "BLE connection interval too small; Firmware upgrade will fail");
        }
      } else if (status == 8) {
        ESP_LOGI(TAG, "decrypt data done");
        std::vector<uint8_t> buffer(3);
        buffer[0] = this->update_bootloader ? BOOTLOADER_UPDATE : SETUP_INITIAL_PARAMS;
        buffer[1] = 2;
        buffer[2] = this->page_idx_qvap;
        this->parent()->write_value(SERVICE_UUID_QVAP, CHARACTERISTIC_UUID, buffer.data(), buffer.size());
      } else if (status == 34) {
        ESP_LOGE(TAG, "Erase failed: please reload and retry.");
      } else if (status == 19) {
        ESP_LOGE(TAG, "Validation failed: please reload and retry.");
        if (this->nb_retries_validation < 1) {
          this->start_qvap_application(true);
          this->nb_retries_validation += 1;
        }
      } else if (status == 51) {
        ESP_LOGE(TAG, "Validation failed (mode): please reload and retry.");
      } else if (status == 35) {
        ESP_LOGE(TAG, "Validation failed: please reload and retry.");
      } else if (status == 82) {
        ESP_LOGE(TAG, "VersionMajor failed: please reload and retry.");
      } else if (status == 98) {
        ESP_LOGE(TAG, "VersionMinor failed: please reload and retry.");
      } else {
        ESP_LOGI(TAG, "undefined status %d - %d", status, data[2]);
      }
    }
  } else if (command == 3) {
    uint8_t err_code = data[1];
    uint8_t err_category = data[2];
    if (err_category == 4) {
      ESP_LOGI(TAG, "Start analysis");
    } else {
      ESP_LOGI(TAG, "Device analysis successful");
      if (data[3] < 9) {
        ESP_LOGI(TAG, "Display brightness reduced");
      }
      if (data[4] & 1) {
        ESP_LOGI(TAG, "Charge limit activated");
      }
      if (!(data[5] & 1)) {
        ESP_LOGI(TAG, "Boost visualization disabled");
      }
      if (data[6] & 1) {
        ESP_LOGI(TAG, "Charge optimization activated");
      }
      if (!(data[7] & 1)) {
        ESP_LOGI(TAG, "Vibration disabled");
      }
      //if (this->brightness_sensor != nullptr)
      //  this->brightness_sensor->publish_state(data[2]);
      //if (this->vibration_sensor != nullptr)
      //  this->vibration_sensor->publish_state(data[5]);
    }
  } else if (command == 4) {
    if (length >= 20) {
      this->heater_runtime_minutes = (data[1] | (data[2] << 8) | (data[3] << 16));
      this->battery_charging_time_minutes = (data[4] | (data[5] << 8) | (data[6] << 16));
      ESP_LOGI(TAG, "Heater Runtime: %d minutes, Battery Charging Time: %d minutes", this->heater_runtime_minutes, this->battery_charging_time_minutes);
      //if (this->heater_runtime_sensor != nullptr)
      //  this->heater_runtime_sensor->publish_state(this->heater_runtime_minutes);
      //if (this->battery_charging_time_sensor != nullptr)
      //  this->battery_charging_time_sensor->publish_state(this->battery_charging_time_minutes);
    }
  } else if (command == 5) {
    if (length >= 17) {
      this->device_prefix = std::string(reinterpret_cast<const char*>(data + 15), 2);
      this->device_name = std::string(reinterpret_cast<const char*>(data + 9), 6);
      ESP_LOGI(TAG, "Device Prefix: %s, Device Name: %s", this->device_prefix.c_str(), this->device_name.c_str());
      //if (this->device_prefix_sensor != nullptr)
      //  this->device_prefix_sensor->publish_state(this->device_prefix);
      //if (this->device_name_sensor != nullptr)
      //  this->device_name_sensor->publish_state(this->device_name);
    }
  } else if (command == 6) {
    if (length >= 5) {
      this->brightness = data[2];
      this->vibration = data[5];
      ESP_LOGI(TAG, "Brightness: %d, Vibration: %s", this->brightness, (this->vibration ? "Enabled" : "Disabled"));
      //if (this->brightness_sensor != nullptr)
      //  this->brightness_sensor->publish_state(this->brightness);
      //if (this->vibration_sensor != nullptr)
      //  this->vibration_sensor->publish_state(this->vibration);
    }
  } else {
    ESP_LOGI(TAG, "Default command %d", command);
  }
}

void QVAPDevice::on_scan_found_device(const esp32_ble_tracker::ESPBTDevice &device) {
  // Check if the device matches the iBeacon UUID or name
  if (device.get_name() == this->ibeacon_name || device.address_str() == this->ibeacon_address) {
    ESP_LOGI(TAG, "Found QVapDevice by iBeacon UUID or name: %s", device.address_str().c_str());
    this->parent()->stop_scan();
    this->parent()->connect(&device);
  }
}

void QVAPDevice::write_page_data_sequence_qvap() {
  // Implement the write page data sequence logic here
}

void QVAPDevice::write_page_data_sequence_qvap() {
  // Implement the write page data sequence logic here
}

void QVAPDevice::start_qvap_application(bool repeat) {
  // Implement the start QVAP application logic here
}

void QVAPDevice::set_target_temperature(float value) {
  int int_value = static_cast<int>(value * 10);
  std::vector<uint8_t> buffer(20, 0);
  buffer[0] = 1;  // Command for setting the target temperature
  buffer[1] = 2;  // MASK_SET_TEMPERATURE_WRITE
  buffer[4] = int_value & 0xFF;
  buffer[5] = (int_value >> 8) & 0xFF;
  this->parent()->write_value(SERVICE_UUID_QVAP, CHARACTERISTIC_UUID, buffer.data(), buffer.size());
  ESP_LOGI(TAG, "Set target temperature to %.1f°C", value);
}

}  // namespace qvap_device
}  // namespace esphome
