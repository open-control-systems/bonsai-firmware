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
#include "ocs_iot/http_pipeline.h"
#include "ocs_iot/json_data_pipeline.h"
#include "ocs_iot/system_pipeline.h"
#include "ocs_pipeline/ds18b20/http_handler.h"
#ifdef CONFIG_OCS_IOT_CONSOLE_PIPELINE_ENABLE
#include "ocs_iot/console_json_pipeline.h"
#endif // CONFIG_OCS_IOT_CONSOLE_PIPELINE_ENABLE

#include "scs/control_pipeline.h"

namespace ocs {
namespace scs {

class ProjectPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    ProjectPipeline();

    //! Start the soil control system.
    status::StatusCode start();

private:
    std::unique_ptr<iot::SystemPipeline> system_pipeline_;
    std::unique_ptr<iot::JsonDataPipeline> json_data_pipeline_;

#ifdef CONFIG_OCS_IOT_CONSOLE_PIPELINE_ENABLE
    std::unique_ptr<iot::ConsoleJsonPipeline> console_pipeline_;
#endif // CONFIG_OCS_IOT_CONSOLE_PIPELINE_ENABLE

    std::unique_ptr<iot::HttpPipeline> http_pipeline_;
    std::unique_ptr<ControlPipeline> control_pipeline_;

    std::unique_ptr<pipeline::ds18b20::HttpHandler> ds18b20_sensor_http_handler_;
};

} // namespace scs
} // namespace ocs
