import esphome.codegen as cg
from esphome.core import CORE
import esphome.config_validation as cv
from esphome.components import ble_client
from esphome.const import CONF_ID


DEPENDENCIES = ['ble_client']
#CODE_OWNERS=["@fsievers22"]

qvap_ns = cg.esphome_ns.namespace("qvap")

QVap = qvap_ns.class_(
    "QVap",
    cg.Component,
    ble_client.BLEClientNode,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(QVap)
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(ble_client.BLE_CLIENT_SCHEMA)
)

#CONF_QVAP_ID = "qvap_id"
#
#QVAP_SCHEMA = cv.Schema(
#    {
#        cv.GenerateID(CONF_QVAP_ID): cv.use_id(QVap),
#    }
#)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)
