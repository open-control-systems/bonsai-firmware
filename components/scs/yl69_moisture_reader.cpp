/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scs/yl69_moisture_reader.h"
#include "ocs_iot/cjson_object_formatter.h"
#include "scs/telemetry.h"

namespace ocs {
namespace app {

YL69MoistureReader::YL69MoistureReader(int threshold, ITelemetryReader& reader)
    : threshold_(threshold)
    , reader_(reader) {
}

status::StatusCode YL69MoistureReader::read(Telemetry& telemetry) {
    const auto code = reader_.read(telemetry);
    if (code != status::StatusCode::OK) {
        return code;
    }

    telemetry.status = telemetry.raw > threshold_ ? SoilStatus::Dry : SoilStatus::Wet;

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
