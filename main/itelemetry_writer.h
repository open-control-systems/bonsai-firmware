#pragma once

#include <memory>

#include "ocs_status/code.h"
#include "telemetry.h"

namespace ocs {
namespace app {

class ITelemetryWriter {
public:
    //! Destroy.
    virtual ~ITelemetryWriter() = default;

    //! Write various soil moisture characteristics.
    [[nodiscard]] virtual status::StatusCode write(const Telemetry& telemetry) = 0;
};

} // namespace app
} // namespace ocs
