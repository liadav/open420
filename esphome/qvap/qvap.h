
#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace custom_sensor {

class CustomSensor : public PollingComponent, public sensor::Sensor {
 public:
  CustomSensor() : PollingComponent(15000) {}  // Poll every 15 seconds

  void setup() override;
  void update() override;

 protected:
  float read_temperature();
};

}  // namespace custom_sensor
}  // namespace esphome
