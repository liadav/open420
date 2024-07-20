
#include "qvap_device.h"
#include "esphome/core/log.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"

namespace esphome {
namespace qvap_device {

static const char *TAG = "qvap_device";
static const uint8_t IBEACON_UUID[16] = { /* Your iBeacon UUID */ };
static const uint128_t QVAP_SERVICE_UUID = {0x00000000, 0x5354, 0x4f52, 0x5a26, 0x4249434b454c};
static const uint128_t QVAP_CHARACTERISTIC_UUID = {0x00000001, 0x5354, 0x4f52, 0x5a26, 0x4249434b454c};
static const uint16_t QVAP_DESCRIPTOR_UUID = 0x2902;  // Client Characteristic Configuration Descriptor UUID

// Initialize ble_scan_params
static esp_ble_scan_params_t ble_scan_params = {
  .scan_type = BLE_SCAN_TYPE_PASSIVE,
  .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
  .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
  .scan_interval = 0x50,
  .scan_window = 0x30,
  .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
};

void QVAPDevice::setup() {
  ESP_LOGCONFIG(TAG, "Setting up QVAPDevice...");
  this->register_for_notify(this->gattc_event_handler);
  this->start_scan();
}

void QVAPDevice::loop() {
  if (!ble_process_pending_ && write_sub_state_ == WRITE_IDLE) {
    uint8_t command_buffer[32];
    switch (state) {
      case STATE_INIT_CONNECTION:
        this->create_command_buffer(0x02, command_buffer);  // INIT_CONNECTION
        esp_ble_gattc_write_char(client_if, conn_id, char_handle_, sizeof(command_buffer), command_buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
        write_sub_state_ = WRITE_IN_PROGRESS;
        this->set_ble_process_pending(true);
        break;
      case STATE_SETUP_INITIAL_PARAMS:
        this->create_command_buffer(0x01, command_buffer);  // SETUP_INITIAL_PARAMS
        esp_ble_gattc_write_char(client_if, conn_id, char_handle_, sizeof(command_buffer), command_buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
        write_sub_state_ = WRITE_IN_PROGRESS;
        this->set_ble_process_pending(true);
        break;
      case STATE_PREPARE_DEVICE:
        this->create_command_buffer(0x04, command_buffer);  // PREPARE_DEVICE
        esp_ble_gattc_write_char(client_if, conn_id, char_handle_, sizeof(command_buffer), command_buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
        write_sub_state_ = WRITE_IN_PROGRESS;
        this->set_ble_process_pending(true);
        break;
      case STATE_ADDITIONAL_SETUP:
        this->create_command_buffer(0x05, command_buffer);  // ADDITIONAL_SETUP
        esp_ble_gattc_write_char(client_if, conn_id, char_handle_, sizeof(command_buffer), command_buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
        write_sub_state_ = WRITE_IN_PROGRESS;
        this->set_ble_process_pending(true);
        break;
      case STATE_FINAL_SETUP:
        this->create_command_buffer(0x06, command_buffer);  // FINAL_SETUP
        esp_ble_gattc_write_char(client_if, conn_id, char_handle_, sizeof(command_buffer), command_buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
        write_sub_state_ = WRITE_IN_PROGRESS;
        this->set_ble_process_pending(true);
        break;
      case STATE_WAIT_FOR_NOTIFICATION:
        if (application_firmware != 0) {
          memset(command_buffer, 0, 32);  // Initialize all bytes to 0
          command_buffer[1] = 6;
          if (application_firmware & 0x01) {
            command_buffer[0] = 0x30;  // BOOTLOADER_UPDATE
          } else {
            command_buffer[0] = 0x01;  // SETUP_INITIAL_PARAMS
          }
          esp_ble_gattc_write_char(client_if, conn_id, char_handle_, sizeof(command_buffer), command_buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
          state = STATE_BOOTLOADER_UPDATE;
          write_sub_state_ = WRITE_IN_PROGRESS;
          this->set_ble_process_pending(true);
        }
        break;
      default:
        break;
    }
  }

  if (write_sub_state_ == WRITE_OK) {
    switch (state) {
      case STATE_INIT_CONNECTION:
        state = STATE_SETUP_INITIAL_PARAMS;
        break;
      case STATE_SETUP_INITIAL_PARAMS:
        state = STATE_PREPARE_DEVICE;
        break;
      case STATE_PREPARE_DEVICE:
        state = STATE_ADDITIONAL_SETUP;
        break;
      case STATE_ADDITIONAL_SETUP:
        state = STATE_FINAL_SETUP;
        break;
      case STATE_FINAL_SETUP:
        state = STATE_WAIT_FOR_NOTIFICATION;
        break;
      default:
        break;
    }
    write_sub_state_ = WRITE_IDLE;
  }
}

void QVAPDevice::start_scan() {
  ESP_LOGI(TAG, "Starting BLE scan...");
  esp_ble_gap_set_scan_params(&ble_scan_params);
}

bool QVAPDevice::is_target_device(const esp_ble_gap_cb_param_t::scan_rst &scan_rst) {
  if (scan_rst.ble_adv[0] == 0x02 && scan_rst.ble_adv[1] == 0x01) {
    if (memcmp(scan_rst.ble_adv + 9, ibeacon_uuid_, 16) == 0) {
      ESP_LOGI(TAG, "Found target iBeacon device!");
      return true;
    }
  }
  return false;
}

void QVAPDevice::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_REG_EVT:
      ESP_LOGI(TAG, "REG_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
      if (param->reg.status == ESP_GATT_OK) {
        this->start_scan();
      }
      break;
    case ESP_GATTC_CONNECT_EVT:
      ESP_LOGI(TAG, "CONNECT_EVT, conn_id %d, gatt_if %d, remote_bda [%02x:%02x:%02x:%02x:%02x:%02x]",
               param->connect.conn_id, gattc_if,
               param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
               param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
      conn_id = param->connect.conn_id;
      client_if = gattc_if;
      esp_ble_gattc_search_service(gattc_if, conn_id, NULL);
      break;
    case ESP_GATTC_SEARCH_RES_EVT:
      ESP_LOGI(TAG, "SEARCH_RES_EVT, conn_id %d, is_primary %d, service uuid: %04x",
               param->search_res.conn_id, param->search_res.is_primary, param->search_res.srvc_id.uuid.uuid.uuid16);
      if (memcmp(param->search_res.srvc_id.uuid.uuid.uuid128, QVAP_SERVICE_UUID, 16) == 0) {
        ESP_LOGI(TAG, "Found QVAP service");
      }
      break;
    case ESP_GATTC_SEARCH_CMPL_EVT:
      ESP_LOGI(TAG, "SEARCH_CMPL_EVT, conn_id %d, status %d", param->search_cmpl.conn_id, param->search_cmpl.status);
      if (param->search_cmpl.status == ESP_GATT_OK) {
        // Confirm if the discovered services include our target service
        bool found = false;
        for (uint16_t i = 0; i < param->search_cmpl.num; ++i) {
          if (memcmp(param->search_cmpl.result[i].uuid.uuid.uuid128, QVAP_SERVICE_UUID, 16) == 0) {
            found = true;
            break;
          }
        }
        if (found) {
          esp_ble_gattc_get_characteristic(gattc_if, param->search_cmpl.conn_id, NULL, NULL);
        } else {
          ESP_LOGI(TAG, "QVAP service not found in search complete.");
        }
      }
      break;
    case ESP_GATTC_GET_CHAR_EVT:
      ESP_LOGI(TAG, "GET_CHAR_EVT, conn_id %d, status %d, char uuid: %04x",
               param->get_char.conn_id, param->get_char.status, param->get_char.char_id.uuid.uuid.uuid16);
      if (param->get_char.status == ESP_GATT_OK) {
        if (memcmp(param->get_char.char_id.uuid.uuid.uuid128, QVAP_CHARACTERISTIC_UUID, 16) == 0) {
          ESP_LOGI(TAG, "Found QVAP characteristic");
          char_handle_ = param->get_char.char_id.char_id.handle;
          esp_ble_gattc_get_descriptor(gattc_if, param->get_char.conn_id, &param->get_char.char_id, NULL);
        }
      }
      break;
    case ESP_GATTC_GET_DESCR_EVT:
      ESP_LOGI(TAG, "GET_DESCR_EVT, conn_id %d, status %d, descr uuid: %04x",
               param->get_descr.conn_id, param->get_descr.status, param->get_descr.descr_id.uuid.uuid.uuid16);
      if (param->get_descr.status == ESP_GATT_OK) {
        if (param->get_descr.descr_id.uuid.uuid.uuid16 == QVAP_DESCRIPTOR_UUID) {
          ESP_LOGI(TAG, "Found QVAP descriptor");
          uint8_t notify_en[2] = {0x01, 0x00};  // Enable notifications
          esp_ble_gattc_write_char_descr(gattc_if, param->get_descr.conn_id, param->get_descr.descr_handle, sizeof(notify_en), notify_en, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
        }
      }
      break;
    case ESP_GATTC_WRITE_DESCR_EVT:
      ESP_LOGI(TAG, "WRITE_DESCR_EVT, conn_id %d, status %d", param->write.conn_id, param->write.status);
      if (param->write.status != ESP_GATT_OK) {
        ESP_LOGE(TAG, "write descriptor failed, status %d", param->write.status);
        break;
      }
      // Trigger the first command after enabling notifications
      state = STATE_INIT_CONNECTION;
      write_sub_state_ = WRITE_IDLE;
      this->set_ble_process_pending(false);
      break;
    case ESP_GATTC_WRITE_CHAR_EVT:
      ESP_LOGI(TAG, "WRITE_CHAR_EVT, conn_id %d, status %d", param->write.conn_id, param->write.status);
      if (param->write.status != ESP_GATT_OK) {
        ESP_LOGE(TAG, "Write char failed, status %d", param->write.status);
        write_sub_state_ = WRITE_FAILED;
        break;
      }
      write_sub_state_ = WRITE_OK;
      this->set_ble_process_pending(false);
      break;
    case ESP_GATTC_NOTIFY_EVT:
      ESP_LOGI(TAG, "NOTIFY_EVT, conn_id %d, value_len %d, value:", param->notify.conn_id, param->notify.value_len);
      esp_log_buffer_hex(TAG, param->notify.value, param->notify.value_len);
      this->parse_response(param->notify.value, param->notify.value_len);
      this->set_ble_process_pending(false);
      break;
    case ESP_GATTC_DISCONNECT_EVT:
      ESP_LOGI(TAG, "DISCONNECT_EVT, conn_id %d, reason %d", param->disconnect.conn_id, param->disconnect.reason);
      state = STATE_IDLE;  // Reset state
      break;
    default:
      break;
  }
}

void QVAPDevice::parse_response(uint8_t *response, size_t length) {
  uint8_t command = response[0];
  
  if (command == 0x02) {  // Command that updates the application firmware status
    application_firmware = response[1];
    char firmware_version[7] = {0};
    char bootloader_version[7] = {0};
    memcpy(firmware_version, &response[2], 6);
    memcpy(bootloader_version, &response[11], 6);
    ESP_LOGI(TAG, "Application Firmware: %s, Bootloader: %s", firmware_version, bootloader_version);
  } else if (command == 0x01 || command == 0x30) {
    if (application_firmware & 0x01 && command != 0x30) {
      uint16_t current_temp = (response[3] << 8) | response[2];
      uint16_t set_temp = (response[5] << 8) | response[4];
      uint8_t boost_temp = response[6];
      uint8_t superboost_temp = response[7];
      uint8_t battery_level = response[8];
      uint16_t auto_shutoff = (response[10] << 8) | response[9];
      uint8_t heater_mode = response[11];
      uint8_t charger_status = response[13];
      uint8_t settings_flags = response[14];

      ESP_LOGI(TAG, "Current Temp: %d, Set Temp: %d, Boost Temp: %d, Superboost Temp: %d, Battery: %d, Auto-shutoff: %d, Heater Mode: %d, Charger: %d, Settings: %d",
               current_temp, set_temp, boost_temp, superboost_temp, battery_level, auto_shutoff, heater_mode, charger_status, settings_flags);
    } else {
      ESP_LOGI(TAG, "_______cmd0x01 - bootloader");
      uint8_t status = response[1];
      if (status == 0x01) {
        // Handle page data write
        ESP_LOGI(TAG, "Page data write request");
        this->write_page_data_sequence_qvap();
      } else if (status == 0x02) {
        // Handle page write completion
        ESP_LOGI(TAG, "Page write complete");
        this->start_qvap_application(true);
      } else if (status == 0x06) {
        ESP_LOGI(TAG, "Connection interval too small; Firmware upgrade may fail");
      } else if (status == 0x08) {
        ESP_LOGI(TAG, "Decrypt data done");
        uint8_t buffer[3];
        buffer[0] = 0x30;
        buffer[1] = 0x02;
        buffer[2] = page_idx_qvap;
        this->send_command(buffer[0], {buffer[1], buffer[2]});
      } else if (status == 0x22) {
        ESP_LOGI(TAG, "Erase failed, please retry");
      } else if (status == 0x13 || status == 0x33 || status == 0x23 || status == 0x52 || status == 0x62) {
        ESP_LOGI(TAG, "Validation failed, please retry");
        if (nb_retries_validation < 1) {
          this->start_qvap_application(true);
          nb_retries_validation++;
        }
      } else {
        ESP_LOGI(TAG, "Undefined status: %d", status);
      }
    }
  } else if (command == 0x03) {
    uint8_t err_code = response[1];
    uint8_t err_category = response[2];
    if (err_category == 0x04) {
      ESP_LOGI(TAG, "Start analysis");
    } else {
      ESP_LOGI(TAG, "Device analysis successful");
      if (response[3] < 9) {
        ESP_LOGI(TAG, "Display brightness reduced");
      }
      if (response[4] & 0x01) {
        ESP_LOGI(TAG, "Charge limit activated");
      }
      if (!(response[5] & 0x01)) {
        ESP_LOGI(TAG, "Boost visualization disabled");
      }
      if (response[6] & 0x01) {
        ESP_LOGI(TAG, "Charge optimization activated");
      }
      if (!(response[7] & 0x01)) {
        ESP_LOGI(TAG, "Vibration disabled");
      }
    }
  } else if (command == 0x04) {
    if (length >= 20) {
      uint32_t heater_runtime_minutes = (response[3] << 16) | (response[2] << 8) | response[1];
      uint32_t battery_charging_time_minutes = (response[6] << 16) | (response[5] << 8) | response[4];
      ESP_LOGI(TAG, "Heater Runtime: %d minutes, Battery Charging Time: %d minutes", heater_runtime_minutes, battery_charging_time_minutes);
    }
  } else if (command == 0x05) {
    if (length >= 17) {
      char device_prefix[3] = {0};
      char device_name[7] = {0};
      memcpy(device_prefix, &response[15], 2);
      memcpy(device_name, &response[9], 6);
      ESP_LOGI(TAG, "Device Prefix: %s, Device Name: %s", device_prefix, device_name);
    }
  } else if (command == 0x06) {
    if (length >= 5) {
      uint8_t brightness = response[2];
      uint8_t vibration = response[5];
      ESP_LOGI(TAG, "Brightness: %d, Vibration: %s", brightness, vibration ? "Enabled" : "Disabled");
    }
  } else {
    ESP_LOGI(TAG, "Unknown command: %d", command);
  }
}

void QVAPDevice::send_command(uint8_t command, const std::vector<uint8_t> &data) {
  uint8_t buffer[32] = {0};
  buffer[0] = command;
  if (!data.empty()) {
    memcpy(buffer + 1, data.data(), data.size());
  }
  esp_ble_gattc_write_char(client_if, conn_id, char_handle_, sizeof(buffer), buffer, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
}

}  // namespace qvap_device
}  // namespace esphome
