#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/http_request/http_request.h"

namespace esphome {
namespace influxdb_writer {

class InfluxDBWriter : public PollingComponent {
   public:
      InfluxDBWriter() : PollingComponent(){};  
      void setup() override;
      void update() override;

      void set_host(const std::string &host) { host_ = host; }
      void set_token(const std::string &token) { token_ = token; }
      void set_bucket(const std::string &bucket) { bucket_ = bucket; }
      void set_org(const std::string &org) { org_ = org; }
      void set_port(const std::string &port) { port_ = port; }
      void set_precision(const std::string &precision) { precision_ = precision; }
      void set_http_request(http_request::HttpRequestComponent *request) { http_request_ = request; }
      void add_sensor_tag(const std::string &sensor, const std::string &tag, const std::string &value);

   protected:
      std::string host_;
      std::string token_;
      std::string bucket_;
      std::string org_;
      std::string port_;
      std::string precision_;

      std::map<std::string, std::map<std::string, std::string>> tags_;

      std::vector<esphome::sensor::Sensor *> sensors_;
      http_request::HttpRequestComponent *http_request_{nullptr};
};

}  // namespace influxdb_writer
}  // namespace esphome
