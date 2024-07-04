#pragma once

#include <vector>

#include "itelemetry_writer.h"
#include "ocs_core/noncopyable.h"

namespace ocs {
namespace app {

class FanoutTelemetryWriter : public ITelemetryWriter, public core::NonCopyable<> {
public:
    //! Propagate telemetry to the underlying writers.
    status::StatusCode write(const Telemetry& telemetry) override;

    //! Add telemetry writer.
    //!
    //! @remarks
    //!  All writers should be added before the main loop is started.
    void add(ITelemetryWriter& writer);

private:
    std::vector<ITelemetryWriter*> writers_;
};

} // namespace app
} // namespace ocs
