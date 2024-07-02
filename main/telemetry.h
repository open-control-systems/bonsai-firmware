#pragma once

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

struct Telemetry {
    SoilStatus status { SoilStatus::None };
    int raw { 0 };
    int voltage { 0 };
};

} // namespace app
} // namespace ocs
