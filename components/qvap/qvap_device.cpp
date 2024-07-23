#include "qvap_device.h"
#include "esphome/core/log.h"

namespace esphome {
namespace qvap {

static const char *TAG = "qvap";

void QVapDevice::setup() {
  this->start_scan();
}

void QVapDevice::loop() {
  if (this->node_state != esp32_ble_tracker::ClientState::ESTABLISHED) {
    return;
  }

  const uint64_t now = millis();
  if (now - this->last_setup_step_time_ >= QVAP_BLE_TIMER_INTERVAL) {
    this->last_setup_step_time_ = now;
    
    switch (this->setup_state_) {
      case INIT_CONNECTION:
        this->init_connection_();
        break;
      case SETUP_INITIAL_PARAMS:
        this->setup_initial_params_();
        break;
      case PREPARE_DEVICE:
        this->prepare_device_();
        break;
      case ADDITIONAL_SETUP:
        this->additional_setup_();
        break;
      case FINAL_SETUP:
        this->final_setup_();
        break;
      case WAIT_FIRMWARE_INFO:
        this->wait_for_firmware_info_();
        break;
      case COMPLETED:
        this->start_periodic_updates_();
        break;
    }
  }
}

void QVapDevice::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_CONNECT_EVT: {
      ESP_LOGI(TAG, "Connected to QVap device");
      this->setup_state_ = INIT_CONNECTION;
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGI(TAG, "Disconnected from QVap device");
      this->setup_state_ = IDLE;
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      auto *chr = this->parent()->get_characteristic(SERVICE_UUID_QVAP, CHARACTERISTIC_UUID);
      if (chr == nullptr) {
        ESP_LOGE(TAG, "No control characteristic found at QVap device");
        break;
      }
      this->char_handle_ = chr->handle;
      
      auto status = esp_ble_gattc_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(), this->char_handle_);
      if (status) {
        ESP_LOGE(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
      }
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      if (param->notify.handle == this->char_handle_) {
        this->parse_response_(param->notify.value, param->notify.value_len);
      }
      break;
    }
    default:
      break;
  }
}

void QVapDevice::start_scan() {
  auto *ble = this->parent()->get_ble();
  ble->set_scan_interval(16);  // 10ms
  ble->set_scan_window(16);    // 10ms

  auto scanner = ble->scan();
  scanner.set_interval(16);
  scanner.set_window(16);
  scanner.set_active(true);
  scanner.set_callback([this](const esp32_ble_tracker::ESPBTDevice &device) {
    return this->parse_device(device);
  });
  scanner.start();
}

bool QVapDevice::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  if (device.address_uint64() != this->address_)
    return false;

  ESP_LOGI(TAG, "Found QVap device!");
  this->connect();
  return true;
}

void QVapDevice::connect() {
  ESP_LOGI(TAG, "Connecting to QVap device...");
  auto status = esp_ble_gattc_open(this->parent()->get_gattc_if(), this->remote_bda_, BLE_ADDR_TYPE_PUBLIC, true);
  if (status) {
    ESP_LOGE(TAG, "esp_ble_gattc_open failed, status=%d", status);
  }
}

void QVapDevice::disconnect() {
  ESP_LOGI(TAG, "Disconnecting from QVap device...");
  auto status = esp_ble_gattc_close(this->parent()->get_gattc_if(), this->parent()->get_conn_id());
  if (status) {
    ESP_LOGE(TAG, "esp_ble_gattc_close failed, status=%d", status);
  }
}

void QVapDevice::init_connection_() {
  uint8_t buffer[20] = {0x02};  // INIT_CONNECTION command
  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->char_handle_,
                           sizeof(buffer), buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  this->setup_state_ = SETUP_INITIAL_PARAMS;
}

void QVapDevice::setup_initial_params_() {
  uint8_t buffer[20] = {0x01};  // SETUP_INITIAL_PARAMS command
  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->char_handle_,
                           sizeof(buffer), buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  this->setup_state_ = PREPARE_DEVICE;
}

void QVapDevice::prepare_device_() {
  uint8_t buffer[20] = {0x04};  // PREPARE_DEVICE command
  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->char_handle_,
                           sizeof(buffer), buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  this->setup_state_ = ADDITIONAL_SETUP;
}

void QVapDevice::additional_setup_() {
  uint8_t buffer[20] = {0x05};  // ADDITIONAL_SETUP command
  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->char_handle_,
                           sizeof(buffer), buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  this->setup_state_ = FINAL_SETUP;
}

void QVapDevice::final_setup_() {
  uint8_t buffer[20] = {0x06};  // FINAL_SETUP command
  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->char_handle_,
                           sizeof(buffer), buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  this->setup_state_ = WAIT_FIRMWARE_INFO;
  ESP_LOGI(TAG, "Sent final setup command, waiting for firmware info");
}

void QVapDevice::wait_for_firmware_info_() {
  // Do nothing here, we're waiting for the response in parse_response_
  ESP_LOGV(TAG, "Waiting for firmware info...");
}

void QVapDevice::send_final_command_() {
  uint8_t buffer[20] = {0};
  if (this->application_firmware_ & 0x01) {  // APPLICATION_FIRMWARE_MASK_APPLICATION
    ESP_LOGI(TAG, "Firmware application valid. Starting periodic updates.");
    buffer[0] = 0x30;  // BOOTLOADER_UPDATE
  } else {
    ESP_LOGI(TAG, "Firmware application invalid. Sending SETUP_INITIAL_PARAMS.");
    buffer[0] = 0x01;  // SETUP_INITIAL_PARAMS
  }
  buffer[1] = 0x06;  // Keeping the same second byte as in the Python script

  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->char_handle_,
                           sizeof(buffer), buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  
  this->setup_state_ = COMPLETED;
}

void QVapDevice::start_periodic_updates_() {
  static uint64_t last_update = 0;
  const uint64_t now = millis();

  // Send periodic updates every 4 seconds
  if (now - last_update >= 4000) {
    last_update = now;

    // Send PREPARE_DEVICE command
    uint8_t prepare_buffer[20] = {0x04};
    esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->char_handle_,
                             sizeof(prepare_buffer), prepare_buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);

    // Send SETUP_INITIAL_PARAMS command
    uint8_t setup_buffer[20] = {0x01, 0x00};
    esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->char_handle_,
                             sizeof(setup_buffer), setup_buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  }
}

void QVapDevice::parse_response_(const uint8_t *data, uint16_t length) {
  if (length < 2)
    return;

  uint8_t command = data[0];
  switch (command) {
    case 0x02: {
      this->application_firmware_ = data[1];
      std::string firmware_version(reinterpret_cast<const char*>(&data[2]), 6);
      std::string bootloader_version(reinterpret_cast<const char*>(&data[11]), 6);
      ESP_LOGI(TAG, "Application Firmware: %s, Bootloader: %s", firmware_version.c_str(), bootloader_version.c_str());
      
      // If we're waiting for firmware info, we can now send the final command
      if (this->setup_state_ == WAIT_FIRMWARE_INFO) {
        this->send_final_command_();
      }
      break;
    }
    case 0x01:
    case 0x30: {
      if (this->application_firmware_ & 0x01) {  // APPLICATION_FIRMWARE_MASK_APPLICATION
        float current_temp = ((data[3] << 8) + data[2]) / 10.0f;
        float set_temp = ((data[5] << 8) + data[4]) / 10.0f;
        float boost_temp = data[6];
        float superboost_temp = data[7];
        float battery_level = data[8];
        uint16_t auto_shutoff = (data[10] << 8) + data[9];
        uint8_t heater_mode = data[11];
        uint8_t charger_status = data[13];
        uint8_t settings_flags = data[14];

        if (this->current_temp_sensor_ != nullptr)
          this->current_temp_sensor_->publish_state(current_temp);
        if (this->set_temp_sensor_ != nullptr)
          this->set_temp_sensor_->publish_state(set_temp);
        if (this->boost_temp_sensor_ != nullptr)
          this->boost_temp_sensor_->publish_state(boost_temp);
        if (this->superboost_temp_sensor_ != nullptr)
          this->superboost_temp_sensor_->publish_state(superboost_temp);
        if (this->battery_level_sensor_ != nullptr)
          this->battery_level_sensor_->publish_state(battery_level);

        ESP_LOGD(TAG, "Current Temp: %.1f, Set Temp: %.1f, Boost: %.1f, Superboost: %.1f, Battery: %.0f%%, Auto-shutoff: %u, Heater Mode: %u, Charger: %u, Settings: 0x%02X",
                 current_temp, set_temp, boost_temp, superboost_temp, battery_level, auto_shutoff, heater_mode, charger_status, settings_flags);
      } else {
        ESP_LOGD(TAG, "Received bootloader response");
        uint8_t status = data[1];
        switch (status) {
          case 1:
            ESP_LOGD(TAG, "Page data write request");
            break;
          case 2:
            ESP_LOGD(TAG, "Page write completed");
            break;
          case 6:
            ESP_LOGD(TAG, "Connection interval: %u", data[2]);
            if (data[2] < 25) {
              ESP_LOGW(TAG, "BLE connection interval too small; Firmware upgrade may fail");
            }
            break;
          case 8:
            ESP_LOGD(TAG, "Decrypt data completed");
            break;
          case 34:
            ESP_LOGW(TAG, "Erase failed: please reload and retry");
            break;
          case 19:
          case 35:
            ESP_LOGW(TAG, "Validation failed: please reload and retry");
            break;
          case 51:
            ESP_LOGW(TAG, "Validation failed (mode): please reload and retry");
            break;
          case 82:
            ESP_LOGW(TAG, "VersionMajor failed: please reload and retry");
            break;
          case 98:
            ESP_LOGW(TAG, "VersionMinor failed: please reload and retry");
            break;
          default:
            ESP_LOGW(TAG, "Undefined bootloader status: %u", status);
            break;
        }
      }
      break;
    }
    case 0x03: {
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
      }
      break;
    }
    case 0x04: {
      if (length >= 7) {
        uint32_t heater_runtime_minutes = data[1] | (data[2] << 8) | (data[3] << 16);
        uint32_t battery_charging_time_minutes = data[4] | (data[5] << 8) | (data[6] << 16);
        ESP_LOGI(TAG, "Heater Runtime: %u minutes, Battery Charging Time: %u minutes", heater_runtime_minutes, battery_charging_time_minutes);
      }
      break;
    }
    case 0x05: {
      if (length >= 17) {
        std::string device_name(reinterpret_cast<const char*>(&data[9]), 6);
        std::string device_prefix(reinterpret_cast<const char*>(&data[15]), 2);
        ESP_LOGI(TAG, "Device Prefix: %s, Device Name: %s", device_prefix.c_str(), device_name.c_str());
      }
      break;
    }
    case 0x06: {
      if (length >= 6) {
        uint8_t brightness = data[2];
        bool vibration_enabled = data[5] & 0x01;
        ESP_LOGI(TAG, "Brightness: %u, Vibration: %s", brightness, vibration_enabled ? "Enabled" : "Disabled");
      }
      break;
    }
    default:
      ESP_LOGW(TAG, "Unknown command: 0x%02X", command);
      break;
  }
}

void QVapDevice::set_target_temperature(float temperature) {
  uint8_t buffer[20] = {0x01, 0x02};  // Command 1, MASK_SET_TEMPERATURE_WRITE
  uint16_t temp_int = temperature * 10;
  buffer[4] = temp_int & 0xFF;
  buffer[5] = (temp_int >> 8) & 0xFF;
  esp_ble_gattc_write_char(this->parent()->get_gattc_if(), this->parent()->get_conn_id(), this->char_handle_,
                           sizeof(buffer), buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
  ESP_LOGI(TAG, "Set target temperature to %.1f°C", temperature);
}

}  // namespace qvap
}  // namespace esphome         ESP_LOGI(TAG
