/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "telemetry_formatter.h"
#include "ocs_iot/cjson_object_formatter.h"

namespace ocs {
namespace app {

void TelemetryFormatter::format(cJSON* json) {
    core::StaticMutex::Lock lock(mu_);

    iot::cJSONObjectFormatter formatter(json);

    formatter.add_number_cs("raw", telemetry_.raw);
    formatter.add_number_cs("voltage", telemetry_.voltage);
    formatter.add_string_ref_cs("status", soil_status_to_str(telemetry_.status));
}

status::StatusCode TelemetryFormatter::write(const Telemetry& telemetry) {
    core::StaticMutex::Lock lock(mu_);

    telemetry_ = telemetry;

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
