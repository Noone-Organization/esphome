#include "influxdb_writer.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"


namespace esphome {
namespace influxdb_writer {

static const char *const TAG = "influxdb_writer";

void InfluxDBWriter::setup() {
  ESP_LOGI(TAG, "Setting up InfluxDBWriter");

  for (auto *sensor : App.get_sensors()) {
    if (sensor != nullptr){
      ESP_LOGD(TAG, "Sensor name: %s, last value: %f", sensor->get_name().c_str(), sensor->state);
      this->sensors_.push_back(sensor);

    }
  }
  ESP_LOGI(TAG, "Amount of sensors detected: %d", this->sensors_.size());
}

void InfluxDBWriter::update() {
  static unsigned long last_sent = 0;

  // Influxdb url
  std::string url = "http://"+this->host_ +":"+this->port_+"/api/v2/write?org="+this->org_ +"&bucket="+this->bucket_+"&precision="+this->precision_;

  // Http body
  std::string line, body;

  for (auto *sensor : this->sensors_) {

    float value = sensor->state;

    // Skip if value is NAN
    if (isnan(value)) continue;

    std::string name = sensor->get_name();

    line = name;
    auto tag_begin = this->tags_.find(name);
    if(tag_begin != this->tags_.end()){
      const auto& sensor_tags = tag_begin->second;
      for (const auto& tag_pair : sensor_tags) {
        line += "," + tag_pair.first + "=" + tag_pair.second;
      }
    }
    line += " value=" + to_string(value) + "\n";
    body += line;
    // Body using line protocol
    //body += name + ",device=esp32 value=" + to_string(value) + "\n";

  }

  ESP_LOGD(TAG, "HTTP Body: %s", body.c_str());

  // Http headers
  std::list<http_request::Header> headers;
  headers.push_back({"Content-Type", "text/plain;charset=utf-8"});  
  headers.push_back({"Authorization", "Token "+this->token_}); 

  // Http POST
  auto container = this->http_request_->post(url, body, headers);

}

void InfluxDBWriter::add_sensor_tag(const std::string &sensor, const std::string &tag, const std::string &value) {
    ESP_LOGD(TAG, "Tag added: sensor=%s, %s=%s", sensor.c_str(), tag.c_str(), value.c_str());
    this->tags_[sensor][tag] = value;
}

}  // namespace influxdb_writer
}  // namespace esphome
