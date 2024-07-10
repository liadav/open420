import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import spi, sensor
from esphome.const import ICON_EMPTY, UNIT_EMPTY

DEPENDENCIES = ["spi"]

empty_spi_sensor_ns = cg.esphome_ns.namespace("empty_spi_sensor")
EmptySPISensor = empty_spi_sensor_ns.class_(
    "EmptySPISensor", cg.PollingComponent, spi.SPIDevice
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        EmptySPISensor,
        unit_of_measurement=UNIT_EMPTY,
        icon=ICON_EMPTY,
        accuracy_decimals=1,
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(spi.spi_device_schema())
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config)
