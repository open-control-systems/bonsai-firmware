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
    , writer_(writer)
    , cond_(mu_) {
}

void SoilMoistureMonitor::start() {
    while (true) {
        relay_turn_on_();
        vTaskDelay(params_.power_on_delay_interval);

        Telemetry telemetry;

        auto status = reader_.read(telemetry);
        if (status == status::StatusCode::OK) {
            status = writer_.write(telemetry);
            if (status != status::StatusCode::OK) {
                ESP_LOGE(log_tag, "failed to write data: code=%s",
                         status::code_to_str(status));
            }
        } else {
            ESP_LOGE(log_tag, "failed to read data: code=%s",
                     status::code_to_str(status));
        }

        relay_turn_off_();

        core::StaticMutex::Lock lock(mu_);
        cond_.wait(params_.read_interval);
    }
}

void SoilMoistureMonitor::reload() {
    core::StaticMutex::Lock lock(mu_);
    cond_.signal();
}

void SoilMoistureMonitor::relay_turn_on_() {
    gpio_set_level(params_.relay_gpio, true);
}

void SoilMoistureMonitor::relay_turn_off_() {
    gpio_set_level(params_.relay_gpio, false);
}

} // namespace app
} // namespace ocs
