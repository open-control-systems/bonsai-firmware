#include <cstring>

#include "esp_log.h"

#include "http_server.h"
#include "telemetry_formatter.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "http-server";

} // namespace

HTTPServer::HTTPServer(const Params& params) {
    config_ = HTTPD_DEFAULT_CONFIG();
    config_.server_port = params.server_port;

    memset(&uri_get_telemetry_, 0, sizeof(uri_get_telemetry_));

    uri_get_telemetry_.uri = "/telemetry";
    uri_get_telemetry_.method = HTTP_GET;
    uri_get_telemetry_.handler = handle_get_;
    uri_get_telemetry_.user_ctx = this;
}

HTTPServer::~HTTPServer() {
    if (handle_) {
        stop();
    }
}

status::StatusCode HTTPServer::write(const Telemetry& telemetry) {
    core::StaticMutex::Lock lock(mu_);
    telemetry_ = telemetry;

    return status::StatusCode::OK;
}

status::StatusCode HTTPServer::start() {
    auto err = httpd_start(&handle_, &config_);
    if (err != ESP_OK) {
        ESP_LOGE(log_tag, "httpd_start(): %s", esp_err_to_name(err));
        return status::StatusCode::Error;
    }

    // Register URI handlers
    err = httpd_register_uri_handler(handle_, &uri_get_telemetry_);
    if (err != ESP_OK) {
        ESP_LOGE(log_tag, "httpd_register_uri_handler(): %s", esp_err_to_name(err));
        return status::StatusCode::Error;
    }

    return status::StatusCode::OK;
}

status::StatusCode HTTPServer::stop() {
    if (handle_) {
        const auto err = httpd_stop(handle_);
        if (err != ESP_OK) {
            ESP_LOGE(log_tag, "httpd_stop(): %s", esp_err_to_name(err));
        }
        handle_ = nullptr;
    }

    return status::StatusCode::OK;
}

esp_err_t HTTPServer::handle_get_(httpd_req_t* req) {
    HTTPServer& self = *static_cast<HTTPServer*>(req->user_ctx);
    self.handle_get_telemetry_(req);

    return ESP_OK;
}

void HTTPServer::handle_get_telemetry_(httpd_req_t* req) {
    TelemetryFormatter formatter;

    {
        core::StaticMutex::Lock lock(mu_);
        formatter.format_json(telemetry_);
    }

    const auto err = httpd_resp_send(req, formatter.c_str(), HTTPD_RESP_USE_STRLEN);
    if (err != ESP_OK) {
        ESP_LOGE(log_tag, "httpd_resp_send(): %s", esp_err_to_name(err));
    }
}

} // namespace app
} // namespace ocs
