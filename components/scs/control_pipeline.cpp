/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ocs_iot/ldr_sensor_json_formatter.h"
#include "ocs_iot/yl69_sensor_json_formatter.h"
#include "ocs_scheduler/high_resolution_timer.h"
#include "ocs_sensor/ldr_sensor_task.h"
#include "ocs_sensor/relay_sensor.h"
#include "ocs_sensor/safe_yl69_sensor_task.h"
#include "ocs_status/code_to_str.h"

#include "scs/control_pipeline.h"

namespace ocs {
namespace scs {

ControlPipeline::ControlPipeline(core::IClock& clock,
                                 storage::StorageBuilder& storage_builder,
                                 system::FanoutRebootHandler& reboot_handler,
                                 scheduler::AsyncTaskScheduler& task_scheduler,
                                 scheduler::TimerStore& timer_store,
                                 diagnostic::BasicCounterHolder& counter_holder,
                                 iot::FanoutJsonFormatter& telemetry_formatter) {
    gpio_config_.reset(new (std::nothrow) GpioConfig());
    configASSERT(gpio_config_);

    adc_store_.reset(new (std::nothrow) io::AdcStore(io::AdcStore::Params {
        .unit = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    }));
    configASSERT(adc_store_);

    counter_storage_ = storage_builder.make("soil_counter");
    configASSERT(counter_storage_);

    fanout_task_.reset(new (std::nothrow) scheduler::FanoutTask());
    configASSERT(fanout_task_);

    yl69_sensor_task_.reset(new (std::nothrow) sensor::SafeYL69SensorTask(
        clock, *adc_store_, *counter_storage_, reboot_handler, task_scheduler,
        timer_store, counter_holder));
    configASSERT(yl69_sensor_task_);

    yl69_sensor_json_formatter_.reset(
        new (std::nothrow) iot::YL69SensorJsonFormatter(yl69_sensor_task_->get_sensor()));
    configASSERT(yl69_sensor_json_formatter_);

    ldr_sensor_task_.reset(new (std::nothrow) sensor::LdrSensorTask(
        *adc_store_, task_scheduler, timer_store));
    configASSERT(ldr_sensor_task_);

    ldr_sensor_json_formatter_.reset(
        new (std::nothrow) iot::LdrSensorJsonFormatter(ldr_sensor_task_->get_sensor()));
    configASSERT(ldr_sensor_json_formatter_);

    fanout_task_->add(*yl69_sensor_task_);
    fanout_task_->add(*ldr_sensor_task_);

    telemetry_formatter.add(*yl69_sensor_json_formatter_);
    telemetry_formatter.add(*ldr_sensor_json_formatter_);
}

status::StatusCode ControlPipeline::start() {
    return fanout_task_->run();
}

scheduler::ITask& ControlPipeline::get_control_task() {
    return *fanout_task_;
}

} // namespace scs
} // namespace ocs
