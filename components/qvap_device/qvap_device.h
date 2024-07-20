#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"

namespace esphome {
namespace qvap_device {

class QVAPDevice : public Component, public ble_client::BLEClientNode {
 public:
  void setup() override;
  void dump_config() override;

  void set_ibeacon_uuid(const std::vector<uint8_t> &uuid) { std::copy(uuid.begin(), uuid.end(), std::begin(ibeacon_uuid_)); }

 protected:
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
  void parse_response(uint8_t *response, size_t length);
  void send_command(uint8_t command, const std::vector<uint8_t> &data = {});

  uint8_t ibeacon_uuid_[16];
  uint16_t conn_id;
  esp_gatt_if_t client_if;
  bool ble_process_pending_;
  uint8_t application_firmware;
  uint16_t char_handle_;

  void set_ble_process_pending(bool pending) { ble_process_pending_ = pending; }

  enum State {
    STATE_IDLE,
    STATE_INIT_CONNECTION,
    STATE_SETUP_INITIAL_PARAMS,
    STATE_PREPARE_DEVICE,
    STATE_ADDITIONAL_SETUP,
    STATE_FINAL_SETUP,
    STATE_WAIT_FOR_NOTIFICATION,
    STATE_BOOTLOADER_UPDATE
  } state;

  enum WriteSubState {
    WRITE_IDLE,
    WRITE_IN_PROGRESS,
    WRITE_OK,
    WRITE_FAILED
  } write_sub_state_;
};

}  // namespace qvap_device
}  // namespace esphome
