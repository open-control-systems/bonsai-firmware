/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scs/telemetry_holder.h"
#include "ocs_iot/cjson_object_formatter.h"

namespace ocs {
namespace app {

status::StatusCode TelemetryHolder::write(const Telemetry& telemetry) {
    core::StaticMutex::Lock lock(mu_);

    telemetry_ = telemetry;

    return status::StatusCode::OK;
}

void TelemetryHolder::format(cJSON* json) {
    core::StaticMutex::Lock lock(mu_);

    iot::cJSONObjectFormatter formatter(json);

    formatter.add_number_cs("sensor_raw", telemetry_.raw);
    formatter.add_number_cs("sensor_voltage", telemetry_.voltage);
    formatter.add_string_ref_cs("sensor_status", soil_status_to_str(telemetry_.status));
}

} // namespace app
} // namespace ocs
