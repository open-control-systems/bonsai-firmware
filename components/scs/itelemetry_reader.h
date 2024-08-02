/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ocs_status/code.h"

#include "scs/telemetry.h"

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
