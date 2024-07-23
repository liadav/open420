import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import (
    CONF_ID,
    CONF_QVAP_ID,
)
from . import QVapDevice

DEPENDENCIES = ["qvap"]

qvap_ns = cg.esphome_ns.namespace("qvap")
QVapClimate = qvap_ns.class_("QVapClimate", climate.Climate, cg.Component)

CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(QVapClimate),
    cv.GenerateID(CONF_QVAP_ID): cv.use_id(QVapDevice),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)

    qvap = await cg.get_variable(config[CONF_QVAP_ID])
    cg.add(var.set_qvap(qvap))
