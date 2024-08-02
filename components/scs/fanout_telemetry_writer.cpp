/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scs/fanout_telemetry_writer.h"

namespace ocs {
namespace scs {

status::StatusCode FanoutTelemetryWriter::write(const Telemetry& telemetry) {
    for (auto& writer : writers_) {
        const auto status = writer->write(telemetry);
        if (status != status::StatusCode::OK) {
            return status;
        }
    }

    return status::StatusCode::OK;
}

void FanoutTelemetryWriter::add(ITelemetryWriter& writer) {
    writers_.emplace_back(&writer);
}

} // namespace scs
} // namespace ocs
