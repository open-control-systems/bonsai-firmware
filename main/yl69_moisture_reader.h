/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "itelemetry_reader.h"
#include "ocs_core/noncopyable.h"

namespace ocs {
namespace app {

class YL69MoistureReader : public ITelemetryReader, public core::NonCopyable<> {
public:
    //! Initialize.
    //!
    //! @params
    //!  - @p threshold after reaching which to consider the soil is wet.
    //!  - @p reader from which to read raw data.
    YL69MoistureReader(int threshold, ITelemetryReader& reader);

    //! Convert raw data based on the provided threshold to the soil moisture status.
    status::StatusCode read(Telemetry& data) override;

private:
    const int threshold_ { 0 };

    ITelemetryReader& reader_;
};

} // namespace app
} // namespace ocs
