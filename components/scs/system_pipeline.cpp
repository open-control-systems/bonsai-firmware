/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ocs_scheduler/high_resolution_timer.h"
#include "ocs_system/default_clock.h"
#include "ocs_system/default_rebooter.h"
#include "ocs_system/delay_rebooter.h"
#include "ocs_system/reboot_task.h"

#include "scs/system_pipeline.h"

namespace ocs {
namespace app {

SystemPipeline::SystemPipeline() {
    flash_initializer_.reset(new (std::nothrow) storage::FlashInitializer());
    configASSERT(flash_initializer_);

    default_clock_.reset(new (std::nothrow) system::DefaultClock());
    configASSERT(default_clock_);

    task_scheduler_.reset(new (std::nothrow) scheduler::AsyncTaskScheduler());
    configASSERT(task_scheduler_);

    timer_store_.reset(new (std::nothrow) scheduler::TimerStore());
    configASSERT(timer_store_);

    fanout_reboot_handler_.reset(new (std::nothrow) system::FanoutRebootHandler());
    configASSERT(fanout_reboot_handler_);

    default_rebooter_.reset(new (std::nothrow)
                                system::DefaultRebooter(*fanout_reboot_handler_));
    configASSERT(default_rebooter_);

    delay_rebooter_.reset(
        new (std::nothrow) system::DelayRebooter(pdMS_TO_TICKS(500), *default_rebooter_));
    configASSERT(delay_rebooter_);

    reboot_task_.reset(new (std::nothrow) system::RebootTask(*delay_rebooter_));
    configASSERT(reboot_task_);

    reboot_task_async_ = task_scheduler_->add(*reboot_task_);
    configASSERT(reboot_task_async_);
}

status::StatusCode SystemPipeline::start() {
    const auto code = timer_store_->start();
    if (code != status::StatusCode::OK) {
        return code;
    }

    task_scheduler_->run();

    return status::StatusCode::OK;
}

core::IClock& SystemPipeline::get_clock() {
    return *default_clock_;
}

scheduler::AsyncTaskScheduler& SystemPipeline::get_task_scheduler() {
    return *task_scheduler_;
}

scheduler::TimerStore& SystemPipeline::get_timer_store() {
    return *timer_store_;
}

scheduler::ITask& SystemPipeline::get_reboot_task() {
    return *reboot_task_async_;
}

system::FanoutRebootHandler& SystemPipeline::get_reboot_handler() {
    return *fanout_reboot_handler_;
}

} // namespace app
} // namespace ocs
