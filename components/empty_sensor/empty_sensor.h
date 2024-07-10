#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace empty_sensor {

class EmptySensor : public sensor::Sensor, public PollingComponent {
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
};

}  // namespace empty_sensor
}  // namespace esphome
