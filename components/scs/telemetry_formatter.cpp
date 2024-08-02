/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "freertos/FreeRTOSConfig.h"

#include "ocs_iot/system_json_formatter.h"

#include "scs/telemetry_formatter.h"

namespace ocs {
namespace app {

TelemetryFormatter::TelemetryFormatter() {
    fanout_formatter_.reset(new (std::nothrow) iot::FanoutJsonFormatter());
    configASSERT(fanout_formatter_);

    system_formatter_.reset(new (std::nothrow) iot::SystemJsonFormatter());
    configASSERT(system_formatter_);

    fanout_formatter_->add(*system_formatter_);
}

void TelemetryFormatter::format(cJSON* json) {
    fanout_formatter_->format(json);
}

iot::FanoutJsonFormatter& TelemetryFormatter::get_fanout_formatter() {
    return *fanout_formatter_;
}

} // namespace app
} // namespace ocs
