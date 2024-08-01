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
#include "ocs_io/igpio.h"
#include "ocs_iot/counter_json_formatter.h"
#include "ocs_iot/default_http_handler.h"
#include "ocs_iot/http_server_pipeline.h"
#include "ocs_iot/system_counter_pipeline.h"
#include "ocs_scheduler/async_task_scheduler.h"
#include "ocs_scheduler/timer_store.h"
#include "ocs_storage/flash_initializer.h"
#include "ocs_storage/storage_builder.h"
#include "ocs_system/fanout_reboot_handler.h"
#include "ocs_system/irebooter.h"
#include "scs/fanout_telemetry_writer.h"
#include "scs/gpio_config.h"
#include "scs/http_command_handler.h"
#include "scs/registration_formatter.h"
#include "scs/soil_moisture_monitor.h"
#include "scs/soil_status_monitor.h"
#include "scs/telemetry_formatter.h"
#include "scs/telemetry_holder.h"

namespace ocs {
namespace app {

class ControlPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    ControlPipeline();

    //! Start the soil control system.
    void start();

private:
    void register_mdns_endpoints_();

    using HttpRegistrationHandler = iot::DefaultHttpHandler<256>;
    using HttpTelemetryHandler = iot::DefaultHttpHandler<512>;

    std::unique_ptr<ITelemetryReader> adc_reader_;
    std::unique_ptr<ITelemetryReader> moisture_reader_;

    std::unique_ptr<storage::FlashInitializer> flash_initializer_;
    std::unique_ptr<iot::HttpServerPipeline> http_server_pipeline_;

    std::unique_ptr<core::IClock> default_clock_;

    std::unique_ptr<scheduler::AsyncTaskScheduler> task_scheduler_;
    std::unique_ptr<scheduler::TimerStore> timer_store_;

    std::unique_ptr<system::FanoutRebootHandler> fanout_reboot_handler_;
    std::unique_ptr<system::IRebooter> default_rebooter_;
    std::unique_ptr<system::IRebooter> delay_rebooter_;

    std::unique_ptr<scheduler::ITask> reboot_task_;
    scheduler::AsyncTaskScheduler::TaskPtr reboot_task_async_;

    std::unique_ptr<FanoutTelemetryWriter> fanout_telemetry_writer_;

    std::unique_ptr<TelemetryHolder> telemetry_holder_;
    std::unique_ptr<TelemetryFormatter> telemetry_formatter_;
    std::unique_ptr<HttpTelemetryHandler> http_telemetry_handler_;

    std::unique_ptr<ITelemetryWriter> console_telemetry_writer_;

    std::unique_ptr<GpioConfig> gpio_config_;

    std::unique_ptr<io::IGpio> default_gpio_;
    std::unique_ptr<io::IGpio> delay_gpio_;
    std::unique_ptr<scheduler::ITask> soil_moisture_task_;
    std::unique_ptr<scheduler::ITask> relay_task_;
    scheduler::AsyncTaskScheduler::TaskPtr relay_task_async_;
    std::unique_ptr<scheduler::ITimer> soil_moisture_timer_;

    std::unique_ptr<HttpCommandHandler> http_command_handler_;

    std::unique_ptr<RegistrationFormatter> registration_formatter_;
    std::unique_ptr<HttpRegistrationHandler> http_registration_handler_;

    std::unique_ptr<storage::StorageBuilder> storage_builder_;
    std::unique_ptr<storage::IStorage> system_counter_storage_;

    std::unique_ptr<iot::CounterJsonFormatter> counter_json_formatter_;
    std::unique_ptr<iot::SystemCounterPipeline> system_counter_pipeline_;

    std::unique_ptr<storage::IStorage> soil_counter_storage_;
    std::unique_ptr<SoilStatusMonitor> soil_status_monitor_;
};

} // namespace app
} // namespace ocs
