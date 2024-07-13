#include "esphome.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include "qvap.h"

namespace esphome {
namespace qvap {


void QVAPComponent::setup() {
  BLEDevice::init("");
  this->pClient = BLEDevice::createClient(BLEAddress(this->address));
  this->connection_stage = STAGE_INIT_CONNECTION;
}

void QVAPComponent::update() {
  if (this->connected) {
    if (this->flip) {
      this->send_command(0x04);  // PREPARE_DEVICE
    } else {
      this->send_command(0x01, {0});  // SETUP_INITIAL_PARAMS
    }
    this->flip = !this->flip;
  } else {
    ESP_LOGI("QVAP", "Connecting");
    this->handle_connection_stage();
  }
}

void QVAPComponent::handle_connection_stage() {
  switch (this->connection_stage) {
    case STAGE_INIT_CONNECTION:
      if (this->pClient->connect(BLEAddress(this->address))) {
        ESP_LOGI("QVAP", "Connected to QVap device.");
        this->connected = true;
        this->pRemoteCharacteristic = this->pClient->getService(this->serviceUUID)->getCharacteristic(this->charUUID);
        this->pRemoteCharacteristic->registerForNotify([this](BLERemoteCharacteristic* characteristic, uint8_t* data, size_t length, bool is_notify) {
          this->notification_handler(characteristic, data, length);
        });
        this->send_command(0x02);  // INIT_CONNECTION
        this->connection_stage = STAGE_SETUP_INITIAL_PARAMS;
      } else {
        ESP_LOGW("QVAP", "Failed to connect to QVap device.");
      }
      break;

    case STAGE_SETUP_INITIAL_PARAMS:
      this->send_command(0x01);  // SETUP_INITIAL_PARAMS
      this->connection_stage = STAGE_PREPARE_DEVICE;
      break;

    case STAGE_PREPARE_DEVICE:
      this->send_command(0x04);  // PREPARE_DEVICE
      this->connection_stage = STAGE_ADDITIONAL_SETUP;
      break;

    case STAGE_ADDITIONAL_SETUP:
      this->send_command(0x05);  // ADDITIONAL_SETUP
      this->connection_stage = STAGE_FINAL_SETUP;
      break;

    case STAGE_FINAL_SETUP:
      this->send_command(0x06);  // FINAL_SETUP
      this->connection_stage = STAGE_COMPLETE;
      break;

    case STAGE_COMPLETE:
      ESP_LOGI("QVAP", "Connection and setup complete.");
      break;
  }
}

void QVAPComponent::send_command(uint8_t command, std::vector<uint8_t> data) {
  uint8_t buffer[20] = {0};
  buffer[0] = command;
  std::copy(data.begin(), data.end(), buffer + 1);
  this->pRemoteCharacteristic->writeValue(buffer, 20, false);
}

void QVAPComponent::notification_handler(BLERemoteCharacteristic* characteristic, uint8_t* data, size_t length) {
  this->parse_response(data, length);
}

void QVAPComponent::parse_response(uint8_t* response, size_t length) {
  uint8_t command = response[0];
  if (command == 2) {
    this->application_firmware = response[1];
    std::string firmware_version(reinterpret_cast<char*>(response + 2), 6);
    std::string bootloader_version(reinterpret_cast<char*>(response + 11), 6);
    ESP_LOGI("QVAP", "Application Firmware: %s, Bootloader: %s", firmware_version.c_str(), bootloader_version.c_str());
  } else if (command == 1 || command == 48) {
    if (this->application_firmware & 1 && command != 48) {
      this->current_temp = (response[3] << 8 | response[2]) / 10.0;
      this->set_temp = (response[5] << 8 | response[4]) / 10.0;
      this->boost_temp = response[6];
      this->superboost_temp = response[7];
      this->battery_level = response[8];
      this->auto_shutoff = (response[10] << 8 | response[9]);
      this->heater_mode = response[11];
      this->charger_status = response[13];
      this->settings_flags = response[14];

      publish_state(this->current_temp, "current_temp");
      publish_state(this->set_temp, "set_temp");
      publish_state(this->boost_temp, "boost_temp");
      publish_state(this->superboost_temp, "superboost_temp");
      publish_state(this->battery_level, "battery_level");
      publish_state(this->auto_shutoff, "auto_shutoff");
      publish_state(this->heater_mode, "heater_mode");
      publish_state(this->charger_status, "charger_status");
      publish_state(this->settings_flags, "settings_flags");

      ESP_LOGI("QVAP", "Current Temp: %.1f, Set Temp: %.1f, Boost Temp: %.1f, Superboost Temp: %.1f, Battery: %d, Auto-shutoff: %d, Heater Mode: %d, Charger: %d, Settings: %d",
               this->current_temp, this->set_temp, this->boost_temp, this->superboost_temp, this->battery_level, this->auto_shutoff, this->heater_mode, this->charger_status, this->settings_flags);
    } else {
      ESP_LOGI("QVAP", "_______cmd0x01 - bootloader");
      uint8_t status = response[1];
      if (status == 1) {
        ESP_LOGI("QVAP", "Page write start request");
        // Implement page write start request logic
      } else if (status == 2) {
        ESP_LOGI("QVAP", "Page write done");
        // Implement page write done logic
      } else if (status == 3) {
        ESP_LOGI("QVAP", "Page data write request");
        // Implement page data write request logic
      } else if (status == 4) {
        ESP_LOGI("QVAP", "Decrypt data done");
        // Implement decrypt data done logic
      } else {
        ESP_LOGI("QVAP", "Undefined status %d - %d", status, response[2]);
      }
    }
  } else if (command == 3) {
    uint8_t err_code = response[1];
    uint8_t err_category = response[2];
    if (err_category == 4) {
      ESP_LOGI("QVAP", "Start analysis");
    } else {
      ESP_LOGI("QVAP", "Device analysis successful");
      if (response[3] < 9) {
        ESP_LOGI("QVAP", "Display brightness reduced");
      }
      if (response[4] & 1) {
        ESP_LOGI("QVAP", "Charge limit activated");
      }
      if (!(response[5] & 1)) {
        ESP_LOGI("QVAP", "Boost visualization disabled");
      }
      if (response[6] & 1) {
        ESP_LOGI("QVAP", "Charge optimization activated");
      }
      if (!(response[7] & 1)) {
        ESP_LOGI("QVAP", "Vibration disabled");
      }
    }
  } else if (command == 4) {
    if (length >= 20) {
      int heater_runtime_minutes = response[1] | (response[2] << 8) | (response[3] << 16);
      int battery_charging_time_minutes = response[4] | (response[5] << 8) | (response[6] << 16);
      ESP_LOGI("QVAP", "Heater Runtime: %d minutes, Battery Charging Time: %d minutes", heater_runtime_minutes, battery_charging_time_minutes);
    }
  } else if (command == 5) {
    if (length >= 17) {
      std::string device_prefix(reinterpret_cast<char*>(response + 15), 2);
      std::string device_name(reinterpret_cast<char*>(response + 9), 6);
      ESP_LOGI("QVAP", "Device Prefix: %s, Device Name: %s", device_prefix.c_str(), device_name.c_str());
    }
  } else if (command == 6) {
    if (length >= 5) {
      uint8_t brightness = response[2];
      uint8_t vibration = response[5];
      ESP_LOGI("QVAP", "Brightness: %d, Vibration: %s", brightness, vibration ? "Enabled" : "Disabled");
    }
  } else {
    ESP_LOGI("QVAP", "Unhandled command %d", command);
  }
}

void QVAPComponent::set_target_temperature(float temperature) {
  uint16_t temp = temperature * 10;
  this->send_command(1, {1, static_cast<uint8_t>(temp & 0xFF), static_cast<uint8_t>((temp >> 8) & 0xFF)});
}

void QVAPComponent::set_boost_temperature(float temperature) {
  this->send_command(1, {2, static_cast<uint8_t>(temperature)});
}

void QVAPComponent::set_superboost_temperature(float temperature) {
  this->send_command(1, {3, static_cast<uint8_t>(temperature)});
}

}  // namespace qvap
}  // namespace esphome
