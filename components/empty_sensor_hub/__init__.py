import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

MULTI_CONF = True

CONF_EMPTY_SENSOR_HUB_ID = "empty_sensor_hub_id"

empty_sensor_hub_ns = cg.esphome_ns.namespace("empty_sensor_hub")

EmptySensorHub = empty_sensor_hub_ns.class_("EmptySensorHub", cg.Component)

HUB_CHILD_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_EMPTY_SENSOR_HUB_ID): cv.use_id(EmptySensorHub),
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(EmptySensorHub),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
