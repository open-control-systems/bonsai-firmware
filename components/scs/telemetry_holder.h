/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ocs_core/noncopyable.h"
#include "ocs_core/static_mutex.h"
#include "ocs_iot/ijson_formatter.h"

#include "scs/itelemetry_writer.h"
#include "scs/telemetry.h"

namespace ocs {
namespace app {

class TelemetryHolder : public ITelemetryWriter,
                        public iot::IJsonFormatter,
                        public core::NonCopyable<> {
public:
    //! Handle telemetry update.
    status::StatusCode write(const Telemetry& telemetry) override;

    //! Format telemetry into @p json.
    void format(cJSON* json) override;

private:
    core::StaticMutex mu_;
    Telemetry telemetry_;
};

} // namespace app
} // namespace ocs
