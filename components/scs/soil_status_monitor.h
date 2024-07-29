/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include "ocs_core/iclock.h"
#include "ocs_core/noncopyable.h"
#include "ocs_diagnostic/basic_counter_holder.h"
#include "ocs_diagnostic/icounter.h"
#include "ocs_diagnostic/state_counter.h"
#include "ocs_storage/istorage.h"
#include "ocs_system/fanout_reboot_handler.h"
#include "scs/itelemetry_writer.h"
#include "scs/telemetry.h"

namespace ocs {
namespace app {

class SoilStatusMonitor : public ITelemetryWriter, public core::NonCopyable<> {
public:
    //! Initialize.
    SoilStatusMonitor(core::IClock& clock,
                      storage::IStorage& storage,
                      system::FanoutRebootHandler& reboot_handler,
                      diagnostic::BasicCounterHolder& counter_holder);

    //! Monitor soil status change in @p telemetry.
    status::StatusCode write(const Telemetry& telemetry) override;

private:
    std::unique_ptr<diagnostic::StateCounter> dry_state_counter_;
    std::unique_ptr<diagnostic::StateCounter> wet_state_counter_;
};

} // namespace app
} // namespace ocs
