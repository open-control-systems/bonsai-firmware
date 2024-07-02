#include "yl69_moisture_reader.h"
#include "ocs_core/cjson_object_formatter.h"
#include "telemetry.h"

namespace ocs {
namespace app {

YL69MoistureReader::YL69MoistureReader(int threshold, ITelemetryReader& reader)
    : threshold_(threshold)
    , reader_(reader) {
}

status::StatusCode YL69MoistureReader::read(Telemetry& telemetry) {
    const auto code = reader_.read(telemetry);
    if (code != status::StatusCode::OK) {
        return code;
    }

    telemetry.status = telemetry.raw > threshold_ ? SoilStatus::Dry : SoilStatus::Wet;

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
