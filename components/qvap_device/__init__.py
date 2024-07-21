import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, esp32_ble_tracker, sensor, number
from esphome.const import CONF_ID, ICON_THERMOMETER, UNIT_CELSIUS, UNIT_PERCENT, UNIT_EMPTY, ICON_EMPTY

DEPENDENCIES = ['ble_client', 'esp32_ble_tracker']

qvap_device_ns = cg.esphome_ns.namespace('qvap_device')
QVAPDevice = qvap_device_ns.class_('QVAPDevice', cg.PollingComponent, ble_client.BLEClientNode, esp32_ble_tracker.ESPBTDeviceListener)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(QVAPDevice),
    cv.Required("ibeacon_name"): cv.string,
    cv.Required("ibeacon_address"): cv.string,
    cv.Optional("current_temp_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_CELSIUS, icon=ICON_THERMOMETER),
    cv.Optional("set_temp_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_CELSIUS, icon=ICON_THERMOMETER),
    cv.Optional("boost_temp_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_CELSIUS, icon=ICON_THERMOMETER),
    cv.Optional("superboost_temp_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_CELSIUS, icon=ICON_THERMOMETER),
    cv.Optional("battery_level_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_PERCENT, icon=ICON_THERMOMETER),
    cv.Optional("auto_shutoff_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY),
    cv.Optional("heater_mode_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY),
    cv.Optional("charger_status_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY),
    cv.Optional("settings_flags_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY),
    cv.Optional("brightness_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY),
    cv.Optional("vibration_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY),
    cv.Optional("device_prefix_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY),
    cv.Optional("device_name_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY),
    cv.Optional("heater_runtime_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY),
    cv.Optional("battery_charging_time_sensor"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY),
    #cv.Optional("target_temp_number"): number.number_schema(
    #    unit_of_measurement=UNIT_CELSIUS,
    #    icon=ICON_THERMOMETER
    #),
}).extend(cv.polling_component_schema('1s'))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_ibeacon_name(config["ibeacon_name"]))
    cg.add(var.set_ibeacon_address(config["ibeacon_address"]))

    if "current_temp_sensor" in config:
        sens = yield sensor.new_sensor(config["current_temp_sensor"])
        cg.add(var.set_current_temp_sensor(sens))

    if "set_temp_sensor" in config:
        sens = yield sensor.new_sensor(config["set_temp_sensor"])
        cg.add(var.set_set_temp_sensor(sens))

    if "boost_temp_sensor" in config:
        sens = yield sensor.new_sensor(config["boost_temp_sensor"])
        cg.add(var.set_boost_temp_sensor(sens))

    if "superboost_temp_sensor" in config:
        sens = yield sensor.new_sensor(config["superboost_temp_sensor"])
        cg.add(var.set_superboost_temp_sensor(sens))

    if "battery_level_sensor" in config:
        sens = yield sensor.new_sensor(config["battery_level_sensor"])
        cg.add(var.set_battery_level_sensor(sens))

    if "auto_shutoff_sensor" in config:
        sens = yield sensor.new_sensor(config["auto_shutoff_sensor"])
        cg.add(var.set_auto_shutoff_sensor(sens))

    if "heater_mode_sensor" in config:
        sens = yield sensor.new_sensor(config["heater_mode_sensor"])
        cg.add(var.set_heater_mode_sensor(sens))

    if "charger_status_sensor" in config:
        sens = yield sensor.new_sensor(config["charger_status_sensor"])
        cg.add(var.set_charger_status_sensor(sens))

    if "settings_flags_sensor" in config:
        sens = yield sensor.new_sensor(config["settings_flags_sensor"])
        cg.add(var.set_settings_flags_sensor(sens))

    if "brightness_sensor" in config:
        sens = yield sensor.new_sensor(config["brightness_sensor"])
        cg.add(var.set_brightness_sensor(sens))

    if "vibration_sensor" in config:
        sens = yield sensor.new_sensor(config["vibration_sensor"])
        cg.add(var.set_vibration_sensor(sens))

    if "device_prefix_sensor" in config:
        sens = yield sensor.new_sensor(config["device_prefix_sensor"])
        cg.add(var.set_device_prefix_sensor(sens))

    if "device_name_sensor" in config:
        sens = yield sensor.new_sensor(config["device_name_sensor"])
        cg.add(var.set_device_name_sensor(sens))

    if "heater_runtime_sensor" in config:
        sens = yield sensor.new_sensor(config["heater_runtime_sensor"])
        cg.add(var.set_heater_runtime_sensor(sens))

    if "battery_charging_time_sensor" in config:
        sens = yield sensor.new_sensor(config["battery_charging_time_sensor"])
        cg.add(var.set_battery_charging_time_sensor(sens))

    #if "target_temp_number" in config:
    #    num = yield number.new_number(config["target_temp_number"])
    #    cg.add(var.set_target_temp_number(num))

    yield cg.register_component(var, config)
    yield ble_client.register_ble_node(var, config)
    yield esp32_ble_tracker.register_ble_listener(var)

