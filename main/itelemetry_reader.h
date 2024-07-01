#pragma once

#include "ocs_status/code.h"
#include "types.h"

namespace ocs {
namespace app {

class ITelemetryReader {
public:
    //! Destroy.
    virtual ~ITelemetryReader() = default;

    //! Read soil moisture characteristics into @p telemetry.
    [[nodiscard]] virtual status::StatusCode read(Telemetry& telemetry) = 0;
};

} // namespace app
} // namespace ocs
