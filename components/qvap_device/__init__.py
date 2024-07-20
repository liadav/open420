import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32_ble_client
from esphome.const import CONF_ID

DEPENDENCIES = ['esp32_ble_client']

qvap_ns = cg.esphome_ns.namespace('qvap_device')
QVAPDevice = qvap_ns.class_('QVAPDevice', cg.Component, esp32_ble_client.ESP32BLEClient)

CONF_IBEACON_UUID = 'ibeacon_uuid'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(QVAPDevice),
    cv.Required(CONF_IBEACON_UUID): cv.hex_uint8_list,
}).extend(cv.COMPONENT_SCHEMA).extend(esp32_ble_client.ESP32_BLE_CLIENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_ibeacon_uuid(config[CONF_IBEACON_UUID]))
    yield cg.register_component(var, config)
    yield esp32_ble_client.register_esp32_ble_client(var, config)
