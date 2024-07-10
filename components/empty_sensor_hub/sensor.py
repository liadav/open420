import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import UNIT_EMPTY, ICON_EMPTY
from . import HUB_CHILD_SCHEMA, CONF_EMPTY_SENSOR_HUB_ID

DEPENDENCIES = ["empty_sensor_hub"]

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY, accuracy_decimals=1
    )
    .extend(HUB_CHILD_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_EMPTY_SENSOR_HUB_ID])
    var = await sensor.new_sensor(config)

    cg.add(paren.register_sensor(var))
