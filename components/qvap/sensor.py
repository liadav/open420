import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_CURRENT_TEMPERATURE,
    CONF_TARGET_TEMPERATURE,
    CONF_BOOST_TEMPERATURE,
    CONF_SUPERBOOST_TEMPERATURE,
    CONF_BATTERY_LEVEL,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_BATTERY,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
)
from . import CONF_QVAP_ID, QVapDevice

DEPENDENCIES = ["qvap"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_QVAP_ID): cv.use_id(QVapDevice),
        cv.Optional(CONF_CURRENT_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_TARGET_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_BOOST_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SUPERBOOST_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_BATTERY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)

async def to_code(config):
    qvap = await cg.get_variable(config[CONF_QVAP_ID])
    
    if CONF_CURRENT_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT_TEMPERATURE])
        cg.add(qvap.set_current_temp_sensor(sens))
    
    if CONF_TARGET_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TARGET_TEMPERATURE])
        cg.add(qvap.set_set_temp_sensor(sens))
    
    if CONF_BOOST_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_BOOST_TEMPERATURE])
        cg.add(qvap.set_boost_temp_sensor(sens))
    
    if CONF_SUPERBOOST_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_SUPERBOOST_TEMPERATURE])
        cg.add(qvap.set_superboost_temp_sensor(sens))
    
    if CONF_BATTERY_LEVEL in config:
        sens = await sensor.new_sensor(config[CONF_BATTERY_LEVEL])
        cg.add(qvap.set_battery_level_sensor(sens))
