#include "yl69_moisture_reader.h"
#include "ocs_core/cjson_object_formatter.h"
#include "types.h"

namespace ocs {
namespace app {

YL69MoistureReader::YL69MoistureReader(int threshold, IJSONReader& reader)
    : threshold_(threshold)
    , reader_(reader) {
}

status::StatusCode YL69MoistureReader::read(core::cJSONSharedBuilder::Ptr& json) {
    if (const auto code = reader_.read(json); code != status::StatusCode::OK) {
        return code;
    }

    const auto object = cJSON_GetObjectItem(
        json.get(), soil_moisture_characteristic_to_str(SoilMoistureCharacteristic::Raw));
    if (!object) {
        return status::StatusCode::NoData;
    }

    const auto raw = cJSON_GetNumberValue(object);
    const auto status = raw > threshold_ ? SoilStatus::Dry : SoilStatus::Wet;

    core::cJSONObjectFormatter formatter(json.get());
    formatter.add_string_ref_cs(
        soil_moisture_characteristic_to_str(SoilMoistureCharacteristic::Status),
        soil_status_to_str(status));

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
