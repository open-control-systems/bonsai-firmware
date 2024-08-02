/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "freertos/FreeRTOSConfig.h"

#include "ocs_core/time.h"
#include "ocs_scheduler/high_resolution_timer.h"

#include "scs/soil_status_monitor.h"

namespace ocs {
namespace scs {

SoilStatusMonitor::SoilStatusMonitor(core::IClock& clock,
                                     storage::IStorage& storage,
                                     system::FanoutRebootHandler& reboot_handler,
                                     scheduler::AsyncTaskScheduler& task_scheduler,
                                     scheduler::TimerStore& timer_store,
                                     diagnostic::BasicCounterHolder& counter_holder) {
    dry_state_task_.reset(new (std::nothrow) diagnostic::StateCounter(
        storage, clock, "c_scs_dry_state", core::Second,
        static_cast<diagnostic::StateCounter::State>(SoilStatus::Dry)));
    configASSERT(dry_state_task_);

    counter_holder.add(*dry_state_task_);
    reboot_handler.add(*dry_state_task_);

    wet_state_task_.reset(new (std::nothrow) diagnostic::StateCounter(
        storage, clock, "c_scs_wet_state", core::Second,
        static_cast<diagnostic::StateCounter::State>(SoilStatus::Wet)));
    configASSERT(wet_state_task_);

    counter_holder.add(*wet_state_task_);
    reboot_handler.add(*wet_state_task_);

    fanout_task_.reset(new (std::nothrow) scheduler::FanoutTask());
    configASSERT(fanout_task_);

    fanout_task_->add(*dry_state_task_);
    fanout_task_->add(*wet_state_task_);

    fanout_task_async_ = task_scheduler.add(*fanout_task_);
    configASSERT(fanout_task_async_);

    task_timer_.reset(new (std::nothrow) scheduler::HighResolutionTimer(
        *fanout_task_async_, "soil-status-monitor", core::Minute * 30));
    configASSERT(task_timer_);

    timer_store.add(*task_timer_);
}

status::StatusCode SoilStatusMonitor::write(const Telemetry& telemetry) {
    dry_state_task_->update(
        static_cast<diagnostic::StateCounter::State>(telemetry.status));

    wet_state_task_->update(
        static_cast<diagnostic::StateCounter::State>(telemetry.status));

    return status::StatusCode::OK;
}

} // namespace scs
} // namespace ocs
