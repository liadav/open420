#include "esphome.h"
#include <string.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

namespace esphome {
namespace qvap {

class QVAPComponent : public PollingComponent, public sensor::Sensor {
 public:
  QVAPComponent() : PollingComponent(4000) {}  // Poll every 4 seconds

  void setup() override;
  void update() override;
  //void set_target_temperature(float temperature);
  //void set_boost_temperature(float temperature);
  //void set_superboost_temperature(float temperature);

 protected:
  //void connect_to_qvap();
  //void handle_connection_stage();
  //void setup_initial_params();
  //void prepare_device();
  //void additional_setup();
  //void final_setup();
  //void read_device_parameters();
  //void send_command(uint8_t command, std::vector<uint8_t> data = {});
  //void notification_handler(BLERemoteCharacteristic* characteristic, uint8_t* data, size_t length);
  //void parse_response(uint8_t* response, size_t length);

  //std::string address;
  //BLEUUID serviceUUID = BLEUUID("00000000-5354-4f52-5a26-4249434b454c");
  //BLEUUID charUUID = BLEUUID("00000001-5354-4f52-5a26-4249434b454c");
  //BLEClient* pClient;
  //BLERemoteCharacteristic* pRemoteCharacteristic;
  //bool connected = false;

  //enum ConnectionStage {
  //  STAGE_INIT_CONNECTION,
  //  STAGE_SETUP_INITIAL_PARAMS,
  //  STAGE_PREPARE_DEVICE,
  //  STAGE_ADDITIONAL_SETUP,
  //  STAGE_FINAL_SETUP,
  //  STAGE_COMPLETE
  //} connection_stage;

  //// Parameters
  //float current_temp = 0;
  //float set_temp = 0;
  //float boost_temp = 0;
  //float superboost_temp = 0;
  //int battery_level = 0;
  //int auto_shutoff = 0;
  //int heater_mode = 0;
  //int charger_status = 0;
  //int settings_flags = 0;

  //uint8_t application_firmware = 0;
  //bool flip = false;  // To flip between two commands during periodic updates
};

}  // namespace empty_sensor
}  // namespace esphome
