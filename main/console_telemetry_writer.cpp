#include "esp_log.h"

#include "console_telemetry_writer.h"
#include "telemetry_formatter.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "console-json-writer";

} // namespace

status::StatusCode ConsoleTelemetryWriter::write(const Telemetry& telemetry) {
    TelemetryFormatter formatter;
    formatter.format_json(telemetry);

    ESP_LOGI(log_tag, "telemetry=%s", formatter.c_str());

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
