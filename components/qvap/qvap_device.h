#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace qvap {

static const uint16_t SERVICE_UUID_QVAP = 0x0000;
static const uint16_t CHARACTERISTIC_UUID = 0x0001;
static const uint64_t QVAP_BLE_TIMER_INTERVAL = 100;  // 100ms

class QVapDevice : public ble_client::BLEClientNode, public Component {
 public:
  float get_setup_priority() const override { return setup_priority::AFTER_BLUETOOTH; }
  void setup() override;
  void loop() override;
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param) override;
  void set_address(uint64_t address) { address_ = address; }
  
  void set_current_temp_sensor(sensor::Sensor *current_temp_sensor) { current_temp_sensor_ = current_temp_sensor; }
  void set_set_temp_sensor(sensor::Sensor *set_temp_sensor) { set_temp_sensor_ = set_temp_sensor; }
  void set_boost_temp_sensor(sensor::Sensor *boost_temp_sensor) { boost_temp_sensor_ = boost_temp_sensor; }
  void set_superboost_temp_sensor(sensor::Sensor *superboost_temp_sensor) { superboost_temp_sensor_ = superboost_temp_sensor; }
  void set_battery_level_sensor(sensor::Sensor *battery_level_sensor) { battery_level_sensor_ = battery_level_sensor; }
  
  void set_target_temperature(float temperature);

 protected:
  void start_scan();
  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device);
  void connect();
  void disconnect();
  void init_connection_();
  void setup_initial_params_();
  void prepare_device_();
  void additional_setup_();
  void final_setup_();
  void wait_for_firmware_info_();
  void send_final_command_();
  void start_periodic_updates_();
  void parse_response_(const uint8_t *data, uint16_t length);

  sensor::Sensor *current_temp_sensor_{nullptr};
  sensor::Sensor *set_temp_sensor_{nullptr};
  sensor::Sensor *boost_temp_sensor_{nullptr};
  sensor::Sensor *superboost_temp_sensor_{nullptr};
  sensor::Sensor *battery_level_sensor_{nullptr};

  uint64_t address_{0};
  uint16_t char_handle_{0};
  esp_bd_addr_t remote_bda_;
  uint8_t application_firmware_{0};
  uint8_t setup_state_{0};
  uint64_t last_setup_step_time_{0};

  enum SetupState {
    IDLE = 0,
    INIT_CONNECTION,
    SETUP_INITIAL_PARAMS,
    PREPARE_DEVICE,
    ADDITIONAL_SETUP,
    FINAL_SETUP,
    WAIT_FIRMWARE_INFO,
    COMPLETED
  };
};

}  // namespace qvap
}  // namespace esphome
