/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
