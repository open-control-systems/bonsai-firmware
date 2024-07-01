#pragma once

#include <unordered_map>

namespace ocs {
namespace app {

//! Known soil statuses.
enum class SoilStatus {
    None,
    Dry,
    Wet,
    Last,
};

//! Convert soil moisture status to human-readable description.
const char* soil_status_to_str(SoilStatus);

//! Parse raw status.
SoilStatus parse_soil_status(unsigned status);

enum class SoilMoistureCharacteristic {
    Raw,
    Voltage,
    Status,
};

const char* soil_moisture_characteristic_to_str(SoilMoistureCharacteristic);

struct Telemetry {
    SoilStatus status { SoilStatus::None };
    int raw { 0 };
    int voltage { 0 };
};

} // namespace app
} // namespace ocs
