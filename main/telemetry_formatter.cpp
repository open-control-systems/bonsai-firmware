/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>

#include "ocs_core/cjson_builder.h"
#include "ocs_core/cjson_object_formatter.h"
#include "telemetry_formatter.h"

namespace ocs {
namespace app {

TelemetryFormatter::TelemetryFormatter() {
    memset(buf_, 0, sizeof(buf_));
}

const char* TelemetryFormatter::c_str() const {
    return buf_;
}

void TelemetryFormatter::format_json(const Telemetry& telemetry) {
    auto json = core::cJSONSharedBuilder::make_json();
    core::cJSONObjectFormatter formatter(json.get());

    formatter.add_number_cs("raw", telemetry.raw);
    formatter.add_number_cs("voltage", telemetry.voltage);
    formatter.add_string_ref_cs("status", soil_status_to_str(telemetry.status));

    cJSON_PrintPreallocated(json.get(), buf_, sizeof(buf_), 0);
}

} // namespace app
} // namespace ocs
