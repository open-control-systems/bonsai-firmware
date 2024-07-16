/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "itelemetry_writer.h"
#include "ocs_core/noncopyable.h"
#include "ocs_core/static_mutex.h"
#include "ocs_iot/ijson_formatter.h"
#include "telemetry.h"

namespace ocs {
namespace app {

class TelemetryFormatter : public ITelemetryWriter,
                           public iot::IJSONFormatter,
                           public core::NonCopyable<> {
public:
    //! Format telemetry into @p json.
    void format(cJSON* json) override;

    //! Handle telemetry update.
    status::StatusCode write(const Telemetry& telemetry) override;

private:
    core::StaticMutex mu_;
    Telemetry telemetry_;
};

} // namespace app
} // namespace ocs
