/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_log.h"

#include "ocs_status/code.h"
#include "ocs_status/code_to_str.h"

#include "scs/control_task.h"

namespace ocs {
namespace scs {

namespace {

const char* log_tag = "control-task";

} // namespace

ControlTask::ControlTask(ITelemetryReader& reader, ITelemetryWriter& writer)
    : reader_(reader)
    , writer_(writer) {
}

status::StatusCode ControlTask::run() {
    Telemetry telemetry;

    auto code = reader_.read(telemetry);
    if (code != status::StatusCode::OK) {
        ESP_LOGE(log_tag, "failed to read data: code=%s", status::code_to_str(code));
        return code;
    }

    code = writer_.write(telemetry);
    if (code != status::StatusCode::OK) {
        ESP_LOGE(log_tag, "failed to write data: code=%s", status::code_to_str(code));
        return code;
    }

    return status::StatusCode::OK;
}

} // namespace scs
} // namespace ocs
