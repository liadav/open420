
#include "custom_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace custom_sensor {

static const char *TAG = "custom_sensor";

void CustomSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Custom Sensor...");
}

void CustomSensor::update() {
  float temperature = read_temperature();
  ESP_LOGD(TAG, "Temperature: %.1f °C", temperature);
  publish_state(temperature);
}

float CustomSensor::read_temperature() {
  // Dummy temperature reading
  return 25.0;
}

}  // namespace custom_sensor
}  // namespace esphome
