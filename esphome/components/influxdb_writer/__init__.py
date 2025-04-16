import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import http_request


CONF_HOST = "host"
CONF_TOKEN = "token"
CONF_BUCKET = "bucket"
CONF_ORG = "org"
CONF_HTTP_REQUEST_ID = "http_request_id"

influxdb_writer_ns = cg.esphome_ns.namespace("influxdb_writer")
InfluxDBWriter = influxdb_writer_ns.class_("InfluxDBWriter", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(InfluxDBWriter),
    cv.Required(CONF_HTTP_REQUEST_ID): cv.use_id(http_request.HttpRequestComponent),
    cv.Required(CONF_HOST): cv.string,
    cv.Required(CONF_TOKEN): cv.string,
    cv.Required(CONF_BUCKET): cv.string,
    cv.Required(CONF_ORG): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    request = await cg.get_variable(config[CONF_HTTP_REQUEST_ID])
    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_token(config[CONF_TOKEN]))
    cg.add(var.set_bucket(config[CONF_BUCKET]))
    cg.add(var.set_org(config[CONF_ORG]))
    cg.add(var.set_http_request(request))
    
    

