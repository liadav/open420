import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32_ble_tracker
from esphome.const import CONF_ID

DEPENDENCIES = ['esp32_ble_tracker']

qvap_ns = cg.esphome_ns.namespace('qvap_device')
QVAPDevice = qvap_ns.class_('QVAPDevice', cg.Component, esp32_ble_tracker.ESP32BLETracker)

CONF_IBEACON_UUID = 'ibeacon_uuid'

# Custom validator for iBeacon UUID
def validate_hex_uint8_list(value):
    value = cv.ensure_list_csv(value)
    if len(value) != 16:
        raise cv.Invalid("iBeacon UUID must be a list of 16 hexadecimal values.")
    for v in value:
        cv.hex_uint8(v)
    return value

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(QVAPDevice),
    cv.Required(CONF_IBEACON_UUID): validate_hex_uint8_list,
}).extend(cv.COMPONENT_SCHEMA).extend(esp32_ble_tracker.ESP32_BLE_TRACKER_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_ibeacon_uuid(config[CONF_IBEACON_UUID]))
    yield cg.register_component(var, config)
    yield esp32_ble_tracker.register_esp32_ble_tracker(var, config)
