/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE
#include "ocs_sensor/soil/analog_relay_sensor_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE
#include "ocs_sensor/soil/analog_sensor_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE

#include "ocs_core/iclock.h"
#include "ocs_core/noncopyable.h"
#include "ocs_fmt/json/fanout_formatter.h"
#include "ocs_io/adc/istore.h"
#include "ocs_scheduler/itask_scheduler.h"
#include "ocs_storage/storage_builder.h"
#include "ocs_system/fanout_reboot_handler.h"

namespace ocs {
namespace bonsai {

class SoilPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    SoilPipeline(io::adc::IStore& adc_store,
                 core::IClock& clock,
                 storage::StorageBuilder& storage_builder,
                 system::FanoutRebootHandler& reboot_handler,
                 scheduler::ITaskScheduler& task_scheduler,
                 fmt::json::FanoutFormatter& telemetry_formatter);

private:
#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE
    std::unique_ptr<sensor::soil::AnalogRelaySensorPipeline>
        analog_relay_sensor_pipeline_;
    std::unique_ptr<fmt::json::IFormatter> analog_relay_sensor_json_formatter_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE
    std::unique_ptr<sensor::soil::AnalogSensorPipeline> analog_sensor_pipeline_;
    std::unique_ptr<fmt::json::IFormatter> analog_sensor_json_formatter_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE
};

} // namespace bonsai
} // namespace ocs
