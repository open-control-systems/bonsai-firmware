/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

#include "ocs_core/noncopyable.h"
#include "scs/itelemetry_writer.h"

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
