/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scs/soil_status_monitor.h"
#include "ocs_core/time.h"

namespace ocs {
namespace app {

SoilStatusMonitor::SoilStatusMonitor(core::IClock& clock,
                                     storage::IStorage& storage,
                                     system::FanoutRebootHandler& reboot_handler,
                                     diagnostic::BasicCounterHolder& counter_holder) {
    dry_state_counter_.reset(new (std::nothrow) diagnostic::StateCounter(
        storage, clock, "c_scs_dry_state", core::Second,
        static_cast<diagnostic::StateCounter::State>(SoilStatus::Dry)));

    counter_holder.add(*dry_state_counter_);
    reboot_handler.add(*dry_state_counter_);

    wet_state_counter_.reset(new (std::nothrow) diagnostic::StateCounter(
        storage, clock, "c_scs_wet_state", core::Second,
        static_cast<diagnostic::StateCounter::State>(SoilStatus::Wet)));

    counter_holder.add(*wet_state_counter_);
    reboot_handler.add(*wet_state_counter_);
}

status::StatusCode SoilStatusMonitor::write(const Telemetry& telemetry) {
    dry_state_counter_->update(
        static_cast<diagnostic::StateCounter::State>(telemetry.status));

    wet_state_counter_->update(
        static_cast<diagnostic::StateCounter::State>(telemetry.status));

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
