#include "telemetry.h"

namespace ocs {
namespace app {

const char* soil_status_to_str(SoilStatus status) {
    switch (status) {
    case SoilStatus::Dry:
        return "dry";

    case SoilStatus::Wet:
        return "wet";

    default:
        break;
    }

    return "<none>";
}

} // namespace app
} // namespace ocs
