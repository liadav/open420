
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, UNIT_CELSIUS, ICON_THERMOMETER, DEVICE_CLASS_TEMPERATURE

DEPENDENCIES = ['sensor']

custom_sensor_ns = cg.esphome_ns.namespace('custom_sensor')
CustomSensor = custom_sensor_ns.class_('CustomSensor', sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = sensor.sensor_schema(CustomSensor, unit_of_measurement=UNIT_CELSIUS, icon=ICON_THERMOMETER, accuracy_decimals=1).extend({
    cv.GenerateID(): cv.declare_id(CustomSensor),
}).extend(cv.polling_component_schema('15s'))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
