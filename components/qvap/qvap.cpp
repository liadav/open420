#include "qvap.h"
#include "esphome/core/log.h"

namespace esphome {
namespace qvap {

static const char *TAG = "qvap.sensor";

void QVap::setup() {}

void QVap::loop() {}

void QVap::update() {}

void QVap::dump_config() { ESP_LOGCONFIG(TAG, "Empty custom sensor"); }

}  // namespace empty_sensor
}  // namespace esphome
