/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace ocs {
namespace scs {

//! Known soil statuses.
enum class SoilStatus {
    None,
    Dry,
    Wet,
    Last,
};

//! Convert soil moisture status to human-readable description.
const char* soil_status_to_str(SoilStatus);

struct Telemetry {
    SoilStatus status { SoilStatus::None };
    int raw { 0 };
    int voltage { 0 };
};

} // namespace scs
} // namespace ocs
