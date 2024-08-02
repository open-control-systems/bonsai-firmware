/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scs/data_pipeline.h"
#include "scs/console_telemetry_writer.h"

namespace ocs {
namespace app {

DataPipeline::DataPipeline(core::IClock& clock,
                           scheduler::AsyncTaskScheduler& task_scheduler,
                           scheduler::TimerStore& timer_store,
                           system::FanoutRebootHandler& reboot_handler) {
    fanout_telemetry_writer_.reset(new (std::nothrow) FanoutTelemetryWriter());
    configASSERT(fanout_telemetry_writer_);

    telemetry_holder_.reset(new (std::nothrow) TelemetryHolder());
    configASSERT(telemetry_holder_);

    fanout_telemetry_writer_->add(*telemetry_holder_);

    telemetry_formatter_.reset(new (std::nothrow) TelemetryFormatter());
    configASSERT(telemetry_formatter_);

    telemetry_formatter_->get_fanout_formatter().add(*telemetry_holder_);

    registration_formatter_.reset(new (std::nothrow) RegistrationFormatter());
    configASSERT(registration_formatter_);

    storage_builder_.reset(new (std::nothrow) storage::StorageBuilder());
    configASSERT(storage_builder_);

    system_counter_storage_ = storage_builder_->make("system_counter");
    configASSERT(system_counter_storage_);

    counter_json_formatter_.reset(new (std::nothrow) iot::CounterJsonFormatter());
    configASSERT(counter_json_formatter_);

    system_counter_pipeline_.reset(new (std::nothrow) iot::SystemCounterPipeline(
        clock, *system_counter_storage_, reboot_handler, task_scheduler, timer_store,
        *counter_json_formatter_));
    configASSERT(system_counter_pipeline_);

    telemetry_formatter_->get_fanout_formatter().add(*counter_json_formatter_);

    soil_counter_storage_ = storage_builder_->make("soil_counter");
    configASSERT(soil_counter_storage_);

    soil_status_monitor_.reset(new (std::nothrow) SoilStatusMonitor(
        clock, *soil_counter_storage_, reboot_handler, task_scheduler, timer_store,
        *counter_json_formatter_));
    configASSERT(soil_status_monitor_);

    fanout_telemetry_writer_->add(*soil_status_monitor_);

    console_telemetry_writer_.reset(new (std::nothrow)
                                        ConsoleTelemetryWriter(*telemetry_formatter_));
    configASSERT(console_telemetry_writer_);

    fanout_telemetry_writer_->add(*console_telemetry_writer_);
}

ITelemetryWriter& DataPipeline::get_telemetry_writer() {
    return *fanout_telemetry_writer_;
}

iot::FanoutJsonFormatter& DataPipeline::get_telemetry_formatter() {
    return telemetry_formatter_->get_fanout_formatter();
}

iot::FanoutJsonFormatter& DataPipeline::get_registration_formatter() {
    return registration_formatter_->get_fanout_formatter();
}

} // namespace app
} // namespace ocs
