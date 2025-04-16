#pragma once

#include "esphome/core/component.h"
#include "esphome/components/http_request/http_request.h"


namespace esphome {
namespace influxdb_writer {

class InfluxDBWriter : public Component {
   public:
      void setup() override;
      void loop() override;

      void set_host(const std::string &host) { host_ = host; }
      void set_token(const std::string &token) { token_ = token; }
      void set_bucket(const std::string &bucket) { bucket_ = bucket; }
      void set_org(const std::string &org) { org_ = org; }
      void set_http_request(http_request::HttpRequestComponent *request) { http_request_ = request; }
 protected:
      std::string host_;
      std::string token_;
      std::string bucket_;
      std::string org_;

      http_request::HttpRequestComponent *http_request_{nullptr};
};

}  // namespace influxdb_writer
}  // namespace esphome
