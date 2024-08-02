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
#include "ocs_io/igpio.h"
#include "ocs_scheduler/async_task_scheduler.h"
#include "ocs_scheduler/timer_store.h"

#include "scs/gpio_config.h"
#include "scs/itelemetry_reader.h"
#include "scs/itelemetry_writer.h"

namespace ocs {
namespace scs {

class ControlPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    ControlPipeline(scheduler::AsyncTaskScheduler& task_scheduler,
                    scheduler::TimerStore& timer_store,
                    ITelemetryWriter& writer);

    //! Start the pipeline.
    status::StatusCode start();

    scheduler::ITask& get_control_task();

private:
    std::unique_ptr<GpioConfig> gpio_config_;

    std::unique_ptr<io::IGpio> default_gpio_;
    std::unique_ptr<io::IGpio> delay_gpio_;

    std::unique_ptr<ITelemetryReader> adc_reader_;
    std::unique_ptr<ITelemetryReader> moisture_reader_;

    std::unique_ptr<scheduler::ITask> control_task_;
    std::unique_ptr<scheduler::ITask> relay_task_;
    scheduler::AsyncTaskScheduler::TaskPtr relay_task_async_;

    scheduler::ITask* task_ { nullptr };
    scheduler::ITask* async_task_ { nullptr };

    std::unique_ptr<scheduler::ITimer> async_task_timer_;
};

} // namespace scs
} // namespace ocs
