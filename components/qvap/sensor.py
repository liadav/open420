import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_MAC_ADDRESS,
    CONF_ID,
)

qvap_ns = cg.esphome_ns.namespace("qvap")

QVap = qvap_ns.class_("QVap", sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = sensor.sensor_schema(
    QVap, unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY, accuracy_decimals=1
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(RuuviTag),
            cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
#            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
#                unit_of_measurement=UNIT_CELSIUS,
#                accuracy_decimals=2,
#                device_class=DEVICE_CLASS_TEMPERATURE,
#                state_class=STATE_CLASS_MEASUREMENT,
#            ),
        }
    )
    .extend(cv.polling_component_schema("4s"))
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))
