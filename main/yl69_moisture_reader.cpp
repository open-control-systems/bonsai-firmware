#include "yl69_moisture_reader.h"
#include "cjson_object_formatter.h"
#include "types.h"

YL69MoistureReader::YL69MoistureReader(int threshold, IJSONReader& reader)
    : threshold_(threshold)
    , reader_(reader) {
}

StatusCode YL69MoistureReader::read(cJSONSharedBuilder::Ptr& json) {
    if (const auto status = reader_.read(json); status != StatusCode::OK) {
        return status;
    }

    const auto object = cJSON_GetObjectItem(
        json.get(), soil_moisture_characteristic_to_str(SoilMoistureCharacteristic::Raw));
    if (!object) {
        return StatusCode::NoData;
    }

    const auto raw = cJSON_GetNumberValue(object);
    const auto status = raw > threshold_ ? SoilStatus::Dry : SoilStatus::Wet;

    cJSONObjectFormatter formatter(json.get());
    formatter.add_string_ref_cs(
        soil_moisture_characteristic_to_str(SoilMoistureCharacteristic::Status),
        soil_status_to_str(status));

    return StatusCode::OK;
}

