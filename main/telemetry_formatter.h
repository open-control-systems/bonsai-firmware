/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ocs_core/noncopyable.h"
#include "telemetry.h"

namespace ocs {
namespace app {

class TelemetryFormatter : public core::NonCopyable<> {
public:
    //! Initialize.
    TelemetryFormatter();

    //! Return formatted telemetry.
    const char* c_str() const;

    //! Format telemetry into JSON string.
    void format_json(const Telemetry& telemetry);

private:
    char buf_[64];
};

} // namespace app
} // namespace ocs
