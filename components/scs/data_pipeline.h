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
#include "ocs_iot/counter_json_formatter.h"
#include "ocs_iot/system_counter_pipeline.h"
#include "ocs_scheduler/async_task_scheduler.h"
#include "ocs_scheduler/timer_store.h"
#include "ocs_storage/storage_builder.h"
#include "ocs_system/fanout_reboot_handler.h"

#include "scs/fanout_telemetry_writer.h"
#include "scs/registration_formatter.h"
#include "scs/soil_status_monitor.h"
#include "scs/telemetry_formatter.h"
#include "scs/telemetry_holder.h"

namespace ocs {
namespace app {

class DataPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    DataPipeline(core::IClock& clock,
                 scheduler::AsyncTaskScheduler& task_scheduler,
                 scheduler::TimerStore& timer_store,
                 system::FanoutRebootHandler& reboot_handler);

    ITelemetryWriter& get_telemetry_writer();
    iot::FanoutJsonFormatter& get_telemetry_formatter();
    iot::FanoutJsonFormatter& get_registration_formatter();

private:
    std::unique_ptr<FanoutTelemetryWriter> fanout_telemetry_writer_;
    std::unique_ptr<ITelemetryWriter> console_telemetry_writer_;

    std::unique_ptr<TelemetryHolder> telemetry_holder_;
    std::unique_ptr<TelemetryFormatter> telemetry_formatter_;

    std::unique_ptr<RegistrationFormatter> registration_formatter_;

    std::unique_ptr<storage::StorageBuilder> storage_builder_;

    std::unique_ptr<storage::IStorage> system_counter_storage_;
    std::unique_ptr<iot::CounterJsonFormatter> counter_json_formatter_;
    std::unique_ptr<iot::SystemCounterPipeline> system_counter_pipeline_;

    std::unique_ptr<storage::IStorage> soil_counter_storage_;
    std::unique_ptr<SoilStatusMonitor> soil_status_monitor_;
};

} // namespace app
} // namespace ocs
