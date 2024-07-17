/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "telemetry_formatter.h"
#include "ocs_iot/system_json_formatter.h"

namespace ocs {
namespace app {

TelemetryFormatter::TelemetryFormatter(iot::IJSONFormatter& formatter) {
    fanout_formatter_.reset(new (std::nothrow) iot::FanoutJSONFormatter());
    fanout_formatter_->add(formatter);

    system_formatter_.reset(new (std::nothrow) iot::SystemJSONFormatter());
    fanout_formatter_->add(*system_formatter_);
}

void TelemetryFormatter::format(cJSON* json) {
    fanout_formatter_->format(json);
}

} // namespace app
} // namespace ocs
