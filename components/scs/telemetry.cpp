/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scs/telemetry.h"

namespace ocs {
namespace app {

const char* soil_status_to_str(SoilStatus status) {
    switch (status) {
    case SoilStatus::Dry:
        return "dry";

    case SoilStatus::Wet:
        return "wet";

    default:
        break;
    }

    return "<none>";
}

} // namespace app
} // namespace ocs