#pragma once

#include "itelemetry_writer.h"
#include "ocs_core/noncopyable.h"
#include "telemetry.h"

namespace ocs {
namespace app {

class ConsoleTelemetryWriter : public ITelemetryWriter, public core::NonCopyable<> {
public:
    //! Write soil moisture data to the console.
    status::StatusCode write(const Telemetry& telemetry) override;
};

} // namespace app
} // namespace ocs
