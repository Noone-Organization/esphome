#include "influxdb_writer.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace influxdb_writer {

static const char *const TAG = "influxdb_writer";

void InfluxDBWriter::setup() {
  ESP_LOGI(TAG, "Setting up InfluxDBWriter");

  if (this->use_ssl) this->url_ = "https://";
  else this->url_ = "http://";

  // Influxdb url
  this->url_ += this->host_ +":"+this->port_+"/api/v2/write?org="+this->org_ +"&bucket="+this->bucket_+"&precision="+this->timestampUnit_;

  // Get all the floating sensors connected
  for (auto *sensor : App.get_sensors()) {
    if (sensor != nullptr){
      this->sensors_.push_back(sensor);
    }
  }
  // Get all binary sensors connected
  for (auto *binary_sensor : App.get_binary_sensors()) {
    if (binary_sensor != nullptr){    
      // Set initial value for binary sensors
      this->binarySensorsStates_[binary_sensor->get_name()] = binary_sensor->state;

      // Callback for binary sensor changes of state
      binary_sensor->add_on_state_callback([this, binary_sensor](bool state) {
        this->binarySensorsStates_[binary_sensor->get_name()] = state;
      });
    }
  }
  // Get all text sensors connected
  for (auto *text_sensor : App.get_text_sensors()) {
    if (text_sensor != nullptr){
      this->textSensors_.push_back(text_sensor);
    }
  }

  // Set Http headers
  this->headers_.push_back({"Content-Type", "text/plain;charset=utf-8"}); 
  // Authorization with API Token for InfluxDB v2
  this->headers_.push_back({"Authorization", "Token "+this->token_}); 
}

void InfluxDBWriter::update() {
  static unsigned long last_sent = 0;
  time_t timestamp = 0;
  bool has_time = false;

  // Variable to save individual measurements
  std::string line;
  // Http body. This will have all the lines (all the measurements)
  std::string body;
  std::string id;
  std::string field;

  // If sntp time is sinchronized, get one timestamp for the measurements
  if (this->time_ != nullptr) {
      timestamp = this->time_->now().timestamp;
      // In case of the clock is not sinchronized yet
      if (timestamp > 1000000000) {
        has_time = true;
        ESP_LOGD(TAG, "Got timestamp: %ld", timestamp);
      }
  }
  else{
    has_time = false;
  }

  // Floating sensors
  for (auto *sensor : this->sensors_) {

    // Gets the float value of the sensor
    float value = sensor->state;

    // Skip if value is NAN
    if (isnan(value)) continue;

    // Gets the name of the sensor
    std::string name = sensor->get_name();

    // Finds the id of the sensor using the name
    auto it = this->sensorNamesWithId_.find(name);
    if (it != this->sensorNamesWithId_.end()) {
      id = it->second;  
    }
    else {
      continue;
    }

    // Start the line protocol with the id of the sensor
    line = id;
    // If tags exist for this sensor, add the key and the name of the configured tag
    auto tag_begin = this->tags_.find(id);
    if(tag_begin != this->tags_.end()){
      const auto& sensor_tags = tag_begin->second;
      for (const auto& tag_pair : sensor_tags) {
        line += "," + tag_pair.first + "=" + tag_pair.second;
      }
    }

    // Checks if the sensor has a custom field name assigned
    auto field_name = this->fieldNames_.find(id);
    field = (field_name != this->fieldNames_.end()) ? field_name->second : "value";

    // Adds the measurement value and the field name if a custom one is available
    line += " " + field + "=" + to_string(value);

    if (has_time == true) {
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

  // Binary sensors
  for (const auto& pair : this->binarySensorsStates_) {
    
    const std::string& name = pair.first;
    bool state = pair.second;

    // Finds id using the name of the binary sensor
    auto it = this->sensorNamesWithId_.find(name);
    if (it != this->sensorNamesWithId_.end()) {
      id = it->second;  
    }
    else {
      continue;
    }
    
    line = id;

    auto tag_begin = this->tags_.find(id);
    if (tag_begin != this->tags_.end()) {
      const auto& sensor_tags = tag_begin->second;
      for (const auto& tag_pair : sensor_tags) {
        line += "," + tag_pair.first + "=" + tag_pair.second;
      }
    }

    auto field_name = this->fieldNames_.find(id);
    field = (field_name != this->fieldNames_.end()) ? field_name->second : "value";

    line += " " + field + "=" + std::to_string(state ? 1 : 0);
    if (has_time) {
      line += " " + std::to_string(timestamp) + "\n";
    } else {
      line += "\n";
    }

    body += line;
  }

  // Text sensors
  for (auto text_sensor : this->textSensors_) {
    std::string value = text_sensor->state;

    // Gets the name of the sensor
    std::string name = text_sensor->get_name();

    // Finds the id of the sensor using the name
    auto it = this->sensorNamesWithId_.find(name);
    if (it != this->sensorNamesWithId_.end()) {
      id = it->second;  
    }
    else {
      continue;
    }

    line = id;

    auto tag_begin = this->tags_.find(id);
    if (tag_begin != this->tags_.end()) {
      const auto& sensor_tags = tag_begin->second;
      for (const auto& tag_pair : sensor_tags) {
        line += "," + tag_pair.first + "=" + tag_pair.second;
      }
    }

    // Checks if the sensor has a custom field name assigned
    auto field_name = this->fieldNames_.find(id);
    field = (field_name != this->fieldNames_.end()) ? field_name->second : "value";

    // Adds the measurement value and the field name if a custom one is available
    line += " " + field + "=\"" + value + "\"";

    if (has_time == true) {
      // Adds the timestamp at the end of the line
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

  // Http POST request
  auto return_code = this->http_request_->post(this->url_, body, this->headers_);

}

// Method to save tags defined for each sensor in the yaml
void InfluxDBWriter::add_sensor_tag(const std::string &sensor, const std::string &tag, const std::string &value) {
  this->tags_[sensor][tag] = value;
}

// Method to save custom field names for each sensor in the yaml
void InfluxDBWriter::set_field_name(const std::string &sensor, const std::string &name) {
  this->fieldNames_[sensor] = name;
}

// Method to save sensors ids and its names
void InfluxDBWriter::add_sensor_name_id(const std::string &sensor_id, const std::string &name) {
  this->sensorNamesWithId_[name] = sensor_id;
}

void InfluxDBWriter::dump_config(){
  ESP_LOGCONFIG(TAG, "Host: %s", this->url_.c_str());
  ESP_LOGCONFIG(TAG, "Port: %s", this->port_.c_str());
  ESP_LOGCONFIG(TAG, "Organization: %s", this->org_.c_str());
  ESP_LOGCONFIG(TAG, "Bucket: %s", this->bucket_.c_str());
  ESP_LOGCONFIG(TAG, "Update interval: %ds", this->update_interval_/1000);
}

}  // namespace influxdb_writer
}  // namespace esphome
