/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_log.h"

#include "console_telemetry_writer.h"
#include "ocs_iot/cjson_builder.h"
#include "telemetry_formatter.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "console-telemetry-writer";

} // namespace

ConsoleTelemetryWriter::ConsoleTelemetryWriter(iot::IJSONFormatter& formatter) {
    formatter_.reset(new (std::nothrow) JSONFormatter(formatter));
}

status::StatusCode ConsoleTelemetryWriter::write(const Telemetry& telemetry) {
    auto json = iot::cJSONUniqueBuilder::make_json();
    formatter_->format(json.get());

    ESP_LOGI(log_tag, "telemetry=%s", formatter_->c_str());

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
