/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_log.h"

#include "console_telemetry_writer.h"
#include "telemetry_formatter.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "console-telemetry-writer";

} // namespace

status::StatusCode ConsoleTelemetryWriter::write(const Telemetry& telemetry) {
    TelemetryFormatter formatter;
    formatter.format_json(telemetry);

    ESP_LOGI(log_tag, "telemetry=%s", formatter.c_str());

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
