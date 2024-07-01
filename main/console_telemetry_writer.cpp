#include <cstdio>
#include <string>

#include "console_telemetry_writer.h"
#include "ocs_core/cjson_builder.h"
#include "ocs_core/cjson_object_formatter.h"

namespace ocs {
namespace app {

status::StatusCode ConsoleTelemetryWriter::write(const Telemetry& telemetry) {
    auto json = core::cJSONSharedBuilder::make_json();
    core::cJSONObjectFormatter formatter(json.get());

    formatter.add_number_cs("raw", telemetry.raw);
    formatter.add_number_cs("voltage", telemetry.voltage);
    formatter.add_string_ref_cs("status", soil_status_to_str(telemetry.status));

    const auto str = std::string(cJSON_PrintUnformatted(json.get()));
    fprintf(stderr, "telemetry=%s\n", str.c_str());

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
