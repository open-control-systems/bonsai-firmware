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
#include "ocs_scheduler/async_task_scheduler.h"
#include "ocs_scheduler/fanout_task.h"
#include "ocs_scheduler/itimer.h"
#include "ocs_scheduler/timer_store.h"
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
                      scheduler::AsyncTaskScheduler& task_scheduler,
                      scheduler::TimerStore& timer_store,
                      diagnostic::BasicCounterHolder& counter_holder);

    //! Monitor soil status change in @p telemetry.
    status::StatusCode write(const Telemetry& telemetry) override;

private:
    std::unique_ptr<diagnostic::StateCounter> dry_state_task_;
    std::unique_ptr<diagnostic::StateCounter> wet_state_task_;

    std::unique_ptr<scheduler::FanoutTask> fanout_task_;
    scheduler::AsyncTaskScheduler::TaskPtr fanout_task_async_;
    std::unique_ptr<scheduler::ITimer> task_timer_;
};

} // namespace app
} // namespace ocs
