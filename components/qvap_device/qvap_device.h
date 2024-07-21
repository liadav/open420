
#include "esphome.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
//#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace qvap_device {

static const char *TAG = "qvap_device";

static const std::string SERVICE_UUID_QVAP = "00000000-5354-4f52-5a26-4249434b454c";
static const std::string SERVICE_UUID_QVAP1 = "00001800-0000-1000-8000-00805f9b34fb";
static const std::string CHARACTERISTIC_UUID = "00000001-5354-4f52-5a26-4249434b454c";
static const std::string CHARACTERISTIC_UUID_SERIAL = "00002a00-0000-1000-8000-00805f9b34fb";

enum Command {
  INIT_CONNECTION = 0x02,
  SETUP_INITIAL_PARAMS = 0x01,
  PREPARE_DEVICE = 0x04,
  ADDITIONAL_SETUP = 0x05,
  FINAL_SETUP = 0x06,
  BOOTLOADER_UPDATE = 0x30
};

enum State {
  STATE_IDLE,
  STATE_INIT_CONNECTION,
  STATE_SETUP_INITIAL_PARAMS,
  STATE_PREPARE_DEVICE,
  STATE_ADDITIONAL_SETUP,
  STATE_FINAL_SETUP,
  STATE_PERIODIC_UPDATE
};

class QVAPDevice : public PollingComponent, public ble_client::BLEClientNode, public esp32_ble_tracker::ESPBTDeviceListener {
 public:
  QVAPDevice() : PollingComponent(1000) {}

  void set_ibeacon_name(const std::string &name);
  void set_ibeacon_address(const std::string &address);

  void setup() override;
  void connect_to_ble();
  void loop() override;
  void init_connection();
  void setup_initial_params();
  void prepare_device();
  void additional_setup();
  void final_setup();
  void final_stage();
  void send_command(Command command, const std::vector<uint8_t> &data = {});
  void start_periodic_updates();
  void on_data(const uint8_t *data, size_t length) override;
  void parse_response(const uint8_t *data, size_t length);
  void on_scan_found_device(const esp32_ble_tracker::ESPBTDevice &device) override;
  void write_page_data_sequence_qvap();
  void start_qvap_application(bool repeat);
  void set_target_temperature(float value);

  // Sensors
  sensor::Sensor *current_temp_sensor = nullptr;
  sensor::Sensor *set_temp_sensor = nullptr;
  sensor::Sensor *boost_temp_sensor = nullptr;
  sensor::Sensor *superboost_temp_sensor = nullptr;
  sensor::Sensor *battery_level_sensor = nullptr;
  sensor::Sensor *auto_shutoff_sensor = nullptr;
  sensor::Sensor *heater_mode_sensor = nullptr;
  sensor::Sensor *charger_status_sensor = nullptr;
  sensor::Sensor *settings_flags_sensor = nullptr;
  sensor::Sensor *brightness_sensor = nullptr;
  sensor::Sensor *vibration_sensor = nullptr;
  sensor::Sensor *device_prefix_sensor = nullptr;
  sensor::Sensor *device_name_sensor = nullptr;
  sensor::Sensor *heater_runtime_sensor = nullptr;
  sensor::Sensor *battery_charging_time_sensor = nullptr;

  // Number
//  number::Number *target_temp_number = nullptr;

 protected:
  std::string ibeacon_name;
  std::string ibeacon_address;
  State state = STATE_IDLE;
  uint8_t application_firmware = 0;
  int data_idx_qvap = 0;
  int page_idx_qvap = 0;
  int page_size_qvap = 0;
  int data_size_firmware_per_packet = 0;
  int binary_number_of_pages = 0;
  int nb_retries_validation = 0;
  bool update_bootloader = false;

  // State Variables
  int current_temp = 0;
  int set_temp = 0;
  int boost_temp = 0;
  int superboost_temp = 0;
  int battery_level = 0;
  int auto_shutoff = 0;
  int heater_mode = 0;
  int charger_status = 0;
  int settings_flags = 0;
  uint8_t brightness = 0;
  uint8_t vibration = 0;
  std::string device_prefix;
  std::string device_name;
  int heater_runtime_minutes = 0;
  int battery_charging_time_minutes = 0;
};

}  // namespace qvap_device
}  // namespace esphome
