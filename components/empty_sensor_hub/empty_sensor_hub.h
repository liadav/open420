#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

namespace esphome {
namespace empty_sensor_hub {

class EmptySensorHub : public Component {
#ifdef USE_SENSOR
 protected:
  std::vector<sensor::Sensor *> sensors_;

 public:
  void register_sensor(sensor::Sensor *obj) { this->sensors_.push_back(obj); }
#endif
#ifdef USE_BINARY_SENSOR
 protected:
  std::vector<binary_sensor::BinarySensor *> binary_sensors_;

 public:
  void register_binary_sensor(binary_sensor::BinarySensor *obj) { this->binary_sensors_.push_back(obj); }
#endif
#ifdef USE_TEXT_SENSOR
 protected:
  std::vector<text_sensor::TextSensor *> text_sensors_;

 public:
  void register_text_sensor(text_sensor::TextSensor *obj) { this->text_sensors_.push_back(obj); }
#endif
  void setup() override;
  void dump_config() override;
};

}  // namespace empty_sensor_hub
}  // namespace esphome
