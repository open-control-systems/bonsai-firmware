/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include "ocs_core/noncopyable.h"
#include "ocs_iot/fanout_json_formatter.h"
#include "ocs_iot/ijson_formatter.h"

namespace ocs {
namespace app {

class TelemetryFormatter : public iot::IJSONFormatter, public core::NonCopyable<> {
public:
    //! Initialize.
    //!
    //! @params
    //!  - @p formatter to format basic telemetry data.
    explicit TelemetryFormatter(iot::IJSONFormatter& formatter);

    //! Format all telemetry data into @p json.
    void format(cJSON* json);

private:
    std::unique_ptr<iot::FanoutJSONFormatter> fanout_formatter_;
    std::unique_ptr<iot::IJSONFormatter> system_formatter_;
};

} // namespace app
} // namespace ocs
