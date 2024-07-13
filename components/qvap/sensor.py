import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, ble_client
from esphome.const import (
    CONF_MAC_ADDRESS,
    CONF_ID,
)

qvap_ns = cg.esphome_ns.namespace("qvap")

QVap = qvap_ns.class_("QVap", ble_client.BLEClient, cg.Component)

#CONFIG_SCHEMA = sensor.sensor_schema(
#    QVap, unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY, accuracy_decimals=1
#)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(QVap),
            cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_device(var, config)

    cg.add(var.set_address(config[CONF_MAC_ADDRESS].as_hex))
