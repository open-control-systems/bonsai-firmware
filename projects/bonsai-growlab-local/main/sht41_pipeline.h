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
#include "ocs_core/time.h"
#include "ocs_fmt/json/fanout_formatter.h"
#include "ocs_fmt/json/iformatter.h"
#include "ocs_io/i2c/istore.h"
#include "ocs_pipeline/httpserver/sht41_handler.h"
#include "ocs_scheduler/itask_scheduler.h"
#include "ocs_sensor/sht41/sensor_pipeline.h"
#include "ocs_storage/storage_builder.h"

namespace ocs {
namespace bonsai {

class SHT41Pipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    SHT41Pipeline(io::i2c::IStore& i2c_store,
                  scheduler::ITaskScheduler& task_scheduler,
                  scheduler::AsyncFuncScheduler& func_scheduler,
                  storage::StorageBuilder& storage_builder,
                  fmt::json::FanoutFormatter& telemetry_formatter,
                  http::Server& server,
                  net::IMdnsDriver& mdns_driver,
                  core::Time read_interval);

private:
    std::unique_ptr<sensor::sht41::SensorPipeline> sensor_pipeline_;
    std::unique_ptr<fmt::json::IFormatter> sensor_json_formatter_;
    std::unique_ptr<pipeline::httpserver::SHT41Handler> sensor_http_handler_;
};

} // namespace bonsai
} // namespace ocs
