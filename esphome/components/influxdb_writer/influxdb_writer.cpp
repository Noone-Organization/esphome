#include "influxdb_writer.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"


namespace esphome {
namespace influxdb_writer {

static const char *const TAG = "influxdb_writer";

void InfluxDBWriter::setup() {
  ESP_LOGI(TAG, "Setting up InfluxDBWriter");

  // Influxdb url
  this->url_ = "http://"+this->host_ +":"+this->port_+"/api/v2/write?org="+this->org_ +"&bucket="+this->bucket_+"&precision="+this->timestampUnit_;

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

  // Variable to save individual measurements
  std::string line;
  // Http body. This will have all the lines (all the measurements)
  std::string body;

  // Go through all sensors detected
  for (auto *sensor : this->sensors_) {

    float value = sensor->state;

    // Skip if value is NAN
    if (isnan(value)) continue;

    std::string name = sensor->get_name();

    // Start the measure with the name of the measure
    line = name;
    // If tags exist for this sensor, add the key and the name of the 
    auto tag_begin = this->tags_.find(name);
    if(tag_begin != this->tags_.end()){
      const auto& sensor_tags = tag_begin->second;
      for (const auto& tag_pair : sensor_tags) {
        line += "," + tag_pair.first + "=" + tag_pair.second;
      }
    }
    line += " value=" + to_string(value);

    // If clock have been sinchronized, get the timestamp
    if (this->time_ != nullptr) {
      time_t timestamp = this->time_->now().timestamp;
      ESP_LOGD(TAG, "Got timestamp: %ld", timestamp);
      // Adds the timestamp at the end of the request
      line += " " + to_string(timestamp) + "\n";
    }
    // If not, don't send timestamp
    else{
      line += "\n";
    }
    // Add measurement to the final body
    body += line;
  }

  ESP_LOGD(TAG, "HTTP Request Body: %s", body.c_str());

  // Http headers
  std::list<http_request::Header> headers;
  headers.push_back({"Content-Type", "text/plain;charset=utf-8"});  
  headers.push_back({"Authorization", "Token "+this->token_}); 

  // Http POST
  auto return_code = this->http_request_->post(this->url_, body, headers);


}

void InfluxDBWriter::add_sensor_tag(const std::string &sensor, const std::string &tag, const std::string &value) {
    ESP_LOGD(TAG, "Tag added: sensor=%s, %s=%s", sensor.c_str(), tag.c_str(), value.c_str());
    this->tags_[sensor][tag] = value;
}

void InfluxDBWriter::dump_config(){

}

}  // namespace influxdb_writer
}  // namespace esphome
