#pragma once

#include "esp_http_server.h"

#include "itelemetry_writer.h"
#include "ocs_core/noncopyable.h"
#include "ocs_core/static_mutex.h"
#include "ocs_status/code.h"

namespace ocs {
namespace app {

class HTTPServer : public ITelemetryWriter, public core::NonCopyable<> {
public:
    //! Initialize.
    HTTPServer();

    //! Destroy.
    ~HTTPServer();

    //! Serve received telemetry.
    status::StatusCode write(const Telemetry& telemetry) override;

    //! Start HTTP server.
    status::StatusCode start();

    //! Stop HTTP server.
    //!
    //! @remarks
    //!  Can be called multiple times.
    status::StatusCode stop();

private:
    static esp_err_t handle_get_(httpd_req_t* req);

    void handle_get_telemetry_(httpd_req_t* req);

    httpd_handle_t handle_ { nullptr };
    httpd_config_t config_;
    httpd_uri_t uri_get_telemetry_;

    core::StaticMutex mu_;
    Telemetry telemetry_;
};

} // namespace app
} // namespace ocs
