
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client
from esphome.const import CONF_ID

DEPENDENCIES = ['esp32_ble_tracker']

qvap_ns = cg.esphome_ns.namespace('qvap_device')
QVAPDevice = qvap_ns.class_('QVAPDevice', cg.Component, ble_client.BLEClientNode)

CONF_IBEACON_UUID = 'ibeacon_uuid'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(QVAPDevice),
    cv.Required(CONF_IBEACON_UUID): cv.uuid,
}).extend(cv.COMPONENT_SCHEMA).extend(ble_client.BLE_CLIENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_ibeacon_uuid(config[CONF_IBEACON_UUID]))
    yield cg.register_component(var, config)
    yield ble_client.register_ble_node(var, config)
