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
#include "ocs_io/adc_store.h"
#include "ocs_io/igpio.h"
#include "ocs_iot/fanout_json_formatter.h"
#include "ocs_scheduler/async_task_scheduler.h"
#include "ocs_scheduler/fanout_task.h"
#include "ocs_scheduler/timer_store.h"
#include "ocs_sensor/basic_sensor_task.h"
#include "ocs_sensor/ldr_sensor.h"
#include "ocs_sensor/yl69_sensor.h"
#include "ocs_storage/istorage.h"
#include "ocs_storage/storage_builder.h"
#include "ocs_system/fanout_reboot_handler.h"

#include "scs/gpio_config.h"

namespace ocs {
namespace scs {

class ControlPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    ControlPipeline(core::IClock& clock,
                    storage::StorageBuilder& storage_builder,
                    system::FanoutRebootHandler& reboot_handler,
                    scheduler::AsyncTaskScheduler& task_scheduler,
                    scheduler::TimerStore& timer_store,
                    diagnostic::BasicCounterHolder& counter_holder,
                    iot::FanoutJsonFormatter& telemetry_formatter);

    //! Start the pipeline.
    status::StatusCode start();

    scheduler::ITask& get_control_task();

private:
    std::unique_ptr<GpioConfig> gpio_config_;
    std::unique_ptr<io::AdcStore> adc_store_;
    std::unique_ptr<storage::IStorage> counter_storage_;
    std::unique_ptr<scheduler::FanoutTask> fanout_task_;

    std::unique_ptr<sensor::BasicSensorTask<sensor::YL69Sensor>> yl69_sensor_task_;
    std::unique_ptr<iot::IJsonFormatter> yl69_sensor_json_formatter_;

    std::unique_ptr<sensor::BasicSensorTask<sensor::LdrSensor>> ldr_sensor_task_;
    std::unique_ptr<iot::IJsonFormatter> ldr_sensor_json_formatter_;
};

} // namespace scs
} // namespace ocs
