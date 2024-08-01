/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ocs_core/noncopyable.h"
#include "ocs_scheduler/itask.h"
#include "scs/itelemetry_reader.h"
#include "scs/itelemetry_writer.h"

namespace ocs {
namespace app {

class SoilMoistureMonitor : public scheduler::ITask, public core::NonCopyable<> {
public:
    //! Initialize.
    SoilMoistureMonitor(ITelemetryReader& reader, ITelemetryWriter& writer);

    //! Monitor soil moisture data.
    status::StatusCode run() override;

private:
    ITelemetryReader& reader_;
    ITelemetryWriter& writer_;
};

} // namespace app
} // namespace ocs
