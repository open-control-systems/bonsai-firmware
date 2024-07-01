#pragma once

#include "itelemetry_reader.h"
#include "ocs_core/noncopyable.h"

namespace ocs {
namespace app {

class YL69MoistureReader : public ITelemetryReader, public core::NonCopyable<> {
public:
    //! Initialize.
    //!
    //! @params
    //!  - @p threshold after reaching which to consider the soil is wet.
    //!  - @p reader from which to read raw data.
    YL69MoistureReader(int threshold, ITelemetryReader& reader);

    //! Convert raw data based on the provided threshold to the soil moisture status.
    status::StatusCode read(Telemetry& data) override;

private:
    const int threshold_ { 0 };

    ITelemetryReader& reader_;
};

} // namespace app
} // namespace ocs
