#include <cstdio>

#include "console_telemetry_writer.h"
#include "telemetry_formatter.h"

namespace ocs {
namespace app {

status::StatusCode ConsoleTelemetryWriter::write(const Telemetry& telemetry) {
    TelemetryFormatter formatter;
    formatter.format_json(telemetry);

    fprintf(stderr, "telemetry=%s\n", formatter.c_str());

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
