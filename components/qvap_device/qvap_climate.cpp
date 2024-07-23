#include "qvap_climate.h"
#include "esphome/core/log.h"

namespace esphome {
namespace qvap {

static const char *TAG = "qvap.climate";

void QVapClimate::setup() {
  // Initialize climate component
  this->target_temperature = NAN;
  this->current_temperature = NAN;
}

void QVapClimate::loop() {
  // Update current temperature from QVapDevice
  if (this->qvap_->current_temp_sensor_ != nullptr) {
    this->current_temperature = this->qvap_->current_temp_sensor_->state;
  }
  
  // Update target temperature from QVapDevice
  if (this->qvap_->set_temp_sensor_ != nullptr) {
    this->target_temperature = this->qvap_->set_temp_sensor_->state;
  }
  
  this->publish_state();
}

void QVapClimate::dump_config() {
  LOG_CLIMATE("", "QVap Climate", this);
}

climate::ClimateTraits QVapClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supports_two_point_target_temperature(false);
  traits.set_visual_min_temperature(10);
  traits.set_visual_max_temperature(40);
  traits.set_visual_temperature_step(0.5);
  return traits;
}

void QVapClimate::control(const climate::ClimateCall &call) {
  if (call.get_target_temperature().has_value()) {
    float temp = *call.get_target_temperature();
    this->qvap_->set_target_temperature(temp);
    this->target_temperature = temp;
  }
  
  this->publish_state();
}

}  // namespace qvap
}  // namespace esphome
