#include "empty_light.h"
#include "esphome/core/log.h"

namespace esphome {
namespace empty_light {

static const char *TAG = "empty_light.light";

void EmptyLightOutput::setup() {}

light::LightTraits EmptyLightOutput::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
  return traits;
}

void EmptyLightOutput::write_state(light::LightState *state) {}

void EmptyLightOutput::dump_config() { ESP_LOGCONFIG(TAG, "Empty custom light"); }

}  // namespace empty_light
}  // namespace esphome
