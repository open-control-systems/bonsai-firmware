#include "esp_log.h"

#include "http_telemetry_writer.h"
#include "telemetry_formatter.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "http-telemetry-writer";

} // namespace

HTTPTelemetryWriter::HTTPTelemetryWriter(net::HTTPServer& server) {
    server.add_GET("/telemetry", [this](httpd_req_t* req) {
        TelemetryFormatter formatter;

        {
            core::StaticMutex::Lock lock(mu_);
            formatter.format_json(telemetry_);
        }

        const auto err = httpd_resp_send(req, formatter.c_str(), HTTPD_RESP_USE_STRLEN);
        if (err != ESP_OK) {
            ESP_LOGE(log_tag, "httpd_resp_send(): %s", esp_err_to_name(err));
            return status::StatusCode::Error;
        }

        return status::StatusCode::OK;
    });
}

status::StatusCode HTTPTelemetryWriter::write(const Telemetry& telemetry) {
    core::StaticMutex::Lock lock(mu_);

    telemetry_ = telemetry;

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
