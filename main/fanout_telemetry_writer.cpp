#include "fanout_telemetry_writer.h"

namespace ocs {
namespace app {

status::StatusCode FanoutTelemetryWriter::write(const Telemetry& telemetry) {
    for (auto& writer : writers_) {
        const auto status = writer->write(telemetry);
        if (status != status::StatusCode::OK) {
            return status;
        }
    }

    return status::StatusCode::OK;
}

void FanoutTelemetryWriter::add(ITelemetryWriter& writer) {
    writers_.emplace_back(&writer);
}

} // namespace app
} // namespace ocs
