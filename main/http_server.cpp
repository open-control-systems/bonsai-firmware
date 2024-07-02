#include <cstring>

#include "esp_netif.h"

#include "http_server.h"
#include "telemetry_formatter.h"

namespace ocs {
namespace app {

HTTPServer::HTTPServer() {
    config_ = HTTPD_DEFAULT_CONFIG();

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
    const auto ret = httpd_start(&handle_, &config_);
    if (ret != ESP_OK) {
        fprintf(stderr, "http-server: failed to start server: err=%d\n", ret);
        return status::StatusCode::Error;
    }

    // Register URI handlers
    httpd_register_uri_handler(handle_, &uri_get_telemetry_);

    return status::StatusCode::OK;
}

status::StatusCode HTTPServer::stop() {
    if (handle_) {
        httpd_stop(handle_);
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

    httpd_resp_send(req, formatter.c_str(), HTTPD_RESP_USE_STRLEN);
}

} // namespace app
} // namespace ocs
