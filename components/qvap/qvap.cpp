#include "qvap.h"

#ifdef USE_ESP32

namespace esphome {
namespace qvap {

static const char *const TAG = "qvap";

static const std::string EMPTY = "";

void BLEClientHID::loop() {
  switch (this->hid_state) {
    case HIDState::BLE_CONNECTED:
      this->read_client_characteristics(); //not instant, finished when hid_state = HIDState::READ_CHARS
      this->hid_state = HIDState::READING_CHARS;
      break;
    case HIDState::READ_CHARS:
      this->configure_hid_client();  // instant
      this->hid_state = HIDState::CONFIGURED;
      this->node_state = espbt::ClientState::ESTABLISHED;
    default:
      break;
  }
}

void BLEClientHID::dump_config() {
  ESP_LOGCONFIG(TAG, "BLE Client HID:");
  ESP_LOGCONFIG(TAG, "  MAC address        : %s",
                this->parent()->address_str().c_str());
}


void QVap::gattc_event_handler(esp_gattc_cb_event_t event,
                                       esp_gatt_if_t gattc_if,
                                       esp_ble_gattc_cb_param_t *param) {
  esp_ble_gattc_cb_param_t *p_data = param;
  switch (event) {
    case ESP_GATTC_CONNECT_EVT: {
      auto ret = esp_ble_set_encryption(param->connect.remote_bda,
                                        ESP_BLE_SEC_ENCRYPT);
      if (ret) {
        ESP_LOGE(TAG, "[%d] [%s] esp_ble_set_encryption error, status=%d",
                 this->parent()->get_connection_index(),
                 this->parent()->address_str().c_str(), ret);
      }
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      ESP_LOGW(TAG, "[%s] Disconnected!",
               this->parent()->address_str().c_str());
      this->status_set_warning();
      break;
    }
    case ESP_GATTC_SEARCH_RES_EVT: {
      ESP_LOGW(TAG, "[%s] Search!",
               this->parent()->address_str().c_str());
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      ESP_LOGD(TAG, "GATTC search finished with status code %d",
                 p_data->search_cmpl.status);
      break;
    }
    case ESP_GATTC_READ_CHAR_EVT:
    case ESP_GATTC_READ_DESCR_EVT: {
      //if (param->read.conn_id != this->parent()->get_conn_id()) break;
      //if (param->read.status != ESP_OK) {
      //  ESP_LOGW(TAG, "GATTC read failed with status code %d",
      //           param->read.status);
      //  break;
      //}
      //GATTReadData *data = new GATTReadData(
      //    param->read.handle, param->read.value, param->read.value_len);
      //this->on_gatt_read_finished(data);
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      //if (param->notify.conn_id != this->parent()->get_conn_id()) break;
      //if (p_data->notify.handle == this->battery_handle) {
      //  uint8_t battery_level = p_data->notify.value[0];
      //  if(this->battery_sensor == nullptr){
      //    break;
      //  }
      //  this->battery_sensor->publish_state(battery_level);
      //} else {
      //  // has to be hid report
      //  this->send_input_report_event(p_data);
      //}
      break;
    }
    default: {
      break;
    }
  }
}



void BLEClientHID::configure_hid_client() {
}

}  // namespace qvap
}  // namespace esphome
#endif
