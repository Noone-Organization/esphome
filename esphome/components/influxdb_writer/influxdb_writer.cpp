#include "influxdb_writer.h"
#include "esphome/core/log.h"

namespace esphome {
namespace influxdb_writer {

static const char *const TAG = "influxdb_writer";

void InfluxDBWriter::setup() {
  ESP_LOGI(TAG, "Setting up InfluxDBWriter");
}

void InfluxDBWriter::loop() {
/*
  static unsigned long last_sent = 0;
  if (millis() - last_sent > 60000 && http_request_ != nullptr) {
    std::string line = "measurement,dispositivo=esp32 valor=42.0";

    std::string url = "http://" + host_ + ":8086/api/v2/write?bucket=" + bucket_ + "&org=" + org_ + "&precision=s";

    std::map<std::string, std::string> headers;
    headers["Authorization"] = "Token " + token_;
    headers["Content-Type"] = "text/plain";

    ESP_LOGI(TAG, "POST a InfluxDB v2 -> %s", url.c_str());
    http_request_->post(
      url,
      line,
      headers
      );

    last_sent = millis();
  }
  */
    static unsigned long last_sent = 0;
    if (millis() - last_sent > 60000 && http_request_ != nullptr) { {
        // Construye la URL para InfluxDB
        std::string url = "http://" + this->host_ + ":8086/api/v2/write?org=Noone&bucket=TestBucket&precision=s";

        // El cuerpo de la solicitud (datos para InfluxDB)
        std::string body = "temperature,device=esp32,city=Sunchales value=15";

        // Crear encabezados para la solicitud (si es necesario)
        std::list<http_request::Header> headers;
        headers.push_back({"Content-Type", "text/plain;charset=utf-8"});  // Ajusta según el tipo necesario
        headers.push_back({"Authorization", "Token TeleAuUi9M9hiQXCouzLvRIUFEW6PqHraoba51ivt7ux0Ve11QMk7ti9nlJafkOmZiUhzH_Jl484xhAJN1ExYw=="});
        // Llamar al método `post` con la URL, el cuerpo y los encabezados
        auto container = this->http_request_->post(url, body, headers);

        // Puedes agregar código para manejar la respuesta (si lo deseas)
        // Ejemplo: container->on_response([](int status, const std::string &response) {...});
    }
    last_sent = millis();
}
}

}  // namespace influxdb_writer
}  // namespace esphome
