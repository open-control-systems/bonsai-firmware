/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ocs_pipeline/jsonfmt/sht41_sensor_formatter.h"

#include "main/sht41_pipeline.h"

namespace ocs {
namespace bonsai {

SHT41Pipeline::SHT41Pipeline(io::i2c::IStore& i2c_store,
                             scheduler::ITaskScheduler& task_scheduler,
                             scheduler::AsyncFuncScheduler& func_scheduler,
                             storage::StorageBuilder& storage_builder,
                             fmt::json::FanoutFormatter& telemetry_formatter,
                             http::IRouter& router,
                             core::Time read_interval) {
    sensor_pipeline_.reset(new (std::nothrow) sensor::sht41::SensorPipeline(
        i2c_store, task_scheduler, storage_builder,
        sensor::sht41::SensorPipeline::Params {
            .read_interval = read_interval,
            .measure_command = sensor::sht41::Sensor::Command::MeasureLowPrecision,
            .heating_command = sensor::sht41::Sensor::Command::ActivateHeater_20mW_100ms,
        }));
    configASSERT(sensor_pipeline_);

    sensor_json_formatter_.reset(
        new (std::nothrow)
            pipeline::jsonfmt::SHT41SensorFormatter(sensor_pipeline_->get_sensor()));
    configASSERT(sensor_json_formatter_);

    telemetry_formatter.add(*sensor_json_formatter_);

    sensor_http_handler_.reset(new (std::nothrow) pipeline::httpserver::SHT41Handler(
        func_scheduler, router, sensor_pipeline_->get_sensor()));
    configASSERT(sensor_http_handler_);
}

} // namespace bonsai
} // namespace ocs
