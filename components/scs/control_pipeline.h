/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#ifdef CONFIG_SCS_SENSOR_LDR_ENABLE
#include "ocs_sensor/ldr/sensor.h"
#endif // CONFIG_SCS_SENSOR_LDR_ENABLE

#ifdef CONFIG_SCS_SENSOR_YL69_ENABLE
#include "ocs_sensor/yl69/sensor.h"
#endif // CONFIG_SCS_SENSOR_YL69_ENABLE

#ifdef CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE
#include "ocs_sensor/ds18b20/sensor.h"
#endif // CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE

#ifdef CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE
#include "ocs_sensor/ds18b20/sensor.h"
#endif // CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE

#include "ocs_core/iclock.h"
#include "ocs_core/noncopyable.h"
#include "ocs_diagnostic/basic_counter_holder.h"
#include "ocs_fmt/json/fanout_formatter.h"
#include "ocs_io/adc_store.h"
#include "ocs_scheduler/async_task_scheduler.h"
#include "ocs_scheduler/fanout_task.h"
#include "ocs_scheduler/timer_store.h"
#include "ocs_sensor/basic_sensor_task.h"
#include "ocs_sensor/ds18b20/store.h"
#include "ocs_storage/istorage.h"
#include "ocs_storage/storage_builder.h"
#include "ocs_system/fanout_reboot_handler.h"

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
                    fmt::json::FanoutFormatter& telemetry_formatter);

    //! Start the pipeline.
    status::StatusCode start();

    sensor::ds18b20::Store& get_ds18b20_store();

private:
    std::unique_ptr<io::AdcStore> adc_store_;
    std::unique_ptr<storage::IStorage> counter_storage_;
    std::unique_ptr<scheduler::FanoutTask> fanout_task_;

#ifdef CONFIG_SCS_SENSOR_YL69_ENABLE
    std::unique_ptr<sensor::BasicSensorTask<sensor::yl69::Sensor>> yl69_sensor_task_;
    std::unique_ptr<fmt::json::IFormatter> yl69_sensor_json_formatter_;
#endif // CONFIG_SCS_SENSOR_YL69_ENABLE

#ifdef CONFIG_SCS_SENSOR_LDR_ENABLE
    std::unique_ptr<sensor::BasicSensorTask<sensor::ldr::Sensor>> ldr_sensor_task_;
    std::unique_ptr<fmt::json::IFormatter> ldr_sensor_json_formatter_;
#endif // CONFIG_SCS_SENSOR_LDR_ENABLE

    std::unique_ptr<storage::IStorage> ds18b20_sensor_storage_;
    std::unique_ptr<sensor::ds18b20::Store> ds18b20_sensor_store_;

#ifdef CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE
    std::unique_ptr<sensor::BasicSensorTask<sensor::ds18b20::Sensor>>
        soil_temperature_sensor_task_;

    std::unique_ptr<fmt::json::IFormatter> soil_temperature_sensor_json_formatter_;
#endif // CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE

#ifdef CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE
    std::unique_ptr<sensor::BasicSensorTask<sensor::ds18b20::Sensor>>
        outside_temperature_sensor_task_;

    std::unique_ptr<fmt::json::IFormatter> outside_temperature_sensor_json_formatter_;
#endif // CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE

    scheduler::ITask* ds18b20_sensor_store_task_ { nullptr };
    std::unique_ptr<scheduler::ITimer> ds18b20_sensor_store_timer_;
};

} // namespace scs
} // namespace ocs
