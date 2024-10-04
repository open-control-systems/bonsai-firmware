/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ocs_core/iclock.h"
#include "ocs_core/noncopyable.h"
#include "ocs_fmt/json/fanout_formatter.h"
#include "ocs_fmt/json/iformatter.h"
#include "ocs_scheduler/idelay_estimator.h"
#include "ocs_scheduler/itask_scheduler.h"
#include "ocs_sensor/ds18b20/sensor_pipeline.h"
#include "ocs_storage/storage_builder.h"

namespace ocs {
namespace bonsai {

class DS18B20Pipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    DS18B20Pipeline(core::IClock& clock,
                    storage::StorageBuilder& storage_builder,
                    scheduler::ITaskScheduler& task_scheduler,
                    fmt::json::FanoutFormatter& telemetry_formatter);

    //! Return the underlying sensors store.
    sensor::ds18b20::Store& get_store();

private:
    std::unique_ptr<storage::IStorage> storage_;
    std::unique_ptr<sensor::ds18b20::Store> store_;

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE
    std::unique_ptr<sensor::ds18b20::SensorPipeline> soil_temperature_pipeline_;
    std::unique_ptr<fmt::json::IFormatter> soil_temperature_json_formatter_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE
    std::unique_ptr<sensor::ds18b20::SensorPipeline> outside_temperature_pipeline_;
    std::unique_ptr<fmt::json::IFormatter> outside_temperature_json_formatter_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE
};

} // namespace bonsai
} // namespace ocs
