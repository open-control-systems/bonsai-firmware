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
#include "scs/soil_moisture_monitor.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "soil-moisture-monitor";

} // namespace

SoilMoistureMonitor::SoilMoistureMonitor(SoilMoistureMonitor::Params params,
                                         ITelemetryReader& reader,
                                         ITelemetryWriter& writer)
    : params_(params)
    , reader_(reader)
    , writer_(writer) {
}

status::StatusCode SoilMoistureMonitor::run() {
    relay_turn_on_();
    vTaskDelay(params_.power_on_delay_interval);

    const auto code = run_();

    relay_turn_off_();

    return code;
}

status::StatusCode SoilMoistureMonitor::run_() {
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

void SoilMoistureMonitor::relay_turn_on_() {
    gpio_set_level(params_.relay_gpio, true);
}

void SoilMoistureMonitor::relay_turn_off_() {
    gpio_set_level(params_.relay_gpio, false);
}

} // namespace app
} // namespace ocs
