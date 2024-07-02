#pragma once

#include "ocs_core/noncopyable.h"
#include "telemetry.h"

namespace ocs {
namespace app {

class TelemetryFormatter : public core::NonCopyable<> {
public:
    //! Initialize.
    TelemetryFormatter();

    //! Return formatted telemetry.
    const char* c_str() const;

    //! Format telemetry into JSON string.
    void format_json(const Telemetry& telemetry);

private:
    char buf_[64];
};

} // namespace app
} // namespace ocs
