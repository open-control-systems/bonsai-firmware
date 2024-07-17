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

namespace ocs {
namespace app {

namespace {

const char* log_tag = "console-telemetry-writer";

} // namespace

ConsoleTelemetryWriter::ConsoleTelemetryWriter(iot::IJSONFormatter& formatter) {
    fanout_formatter_.reset(new (std::nothrow) iot::FanoutJSONFormatter());
    fanout_formatter_->add(formatter);

    json_formatter_.reset(new (std::nothrow) JSONFormatter());
    fanout_formatter_->add(*json_formatter_);
}

status::StatusCode ConsoleTelemetryWriter::write(const Telemetry& telemetry) {
    auto json = iot::cJSONUniqueBuilder::make_json();
    fanout_formatter_->format(json.get());

    ESP_LOGI(log_tag, "telemetry=%s", json_formatter_->c_str());

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
