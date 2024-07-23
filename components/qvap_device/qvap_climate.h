#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "qvap_device.h"

namespace esphome {
namespace qvap {

class QVapClimate : public climate::Climate, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  
  void set_qvap(QVapDevice *qvap) { qvap_ = qvap; }
  
  climate::ClimateTraits traits() override;
  
 protected:
  void control(const climate::ClimateCall &call) override;
  
  QVapDevice *qvap_;
};

}  // namespace qvap
}  // namespace esphome
