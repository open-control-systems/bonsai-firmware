#pragma once

#include "itelemetry_writer.h"
#include "ocs_core/noncopyable.h"
#include "ocs_core/static_mutex.h"
#include "ocs_net/http_server.h"

namespace ocs {
namespace app {

class HTTPTelemetryWriter : public ITelemetryWriter, public core::NonCopyable<> {
public:
    //! Initialize.
    //!
    //! @params
    //!  - @p server to register telemetry serving over HTTP.
    explicit HTTPTelemetryWriter(net::HTTPServer& server);

    //! Serve received telemetry over HTTP.
    [[nodiscard]] status::StatusCode write(const Telemetry& telemetry) override;

private:
    core::StaticMutex mu_;
    Telemetry telemetry_;
};

} // namespace app
} // namespace ocs
