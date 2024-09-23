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
#include "ocs_pipeline/ds18b20/http_handler.h"
#include "ocs_pipeline/http_pipeline.h"
#include "ocs_pipeline/json_data_pipeline.h"
#include "ocs_pipeline/system_pipeline.h"
#ifdef CONFIG_OCS_PIPELINE_CONSOLE_PIPELINE_ENABLE
#include "ocs_pipeline/console_json_pipeline.h"
#endif // CONFIG_OCS_PIPELINE_CONSOLE_PIPELINE_ENABLE

#include "control_pipeline.h"

namespace ocs {
namespace bonsai {

class ProjectPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    ProjectPipeline();

    //! Start the soil control system.
    status::StatusCode start();

private:
    std::unique_ptr<pipeline::SystemPipeline> system_pipeline_;
    std::unique_ptr<pipeline::JsonDataPipeline> json_data_pipeline_;

#ifdef CONFIG_OCS_PIPELINE_CONSOLE_PIPELINE_ENABLE
    std::unique_ptr<pipeline::ConsoleJsonPipeline> console_pipeline_;
#endif // CONFIG_OCS_PIPELINE_CONSOLE_PIPELINE_ENABLE

    std::unique_ptr<pipeline::HttpPipeline> http_pipeline_;
    std::unique_ptr<ControlPipeline> control_pipeline_;

    std::unique_ptr<pipeline::ds18b20::HttpHandler> ds18b20_sensor_http_handler_;
};

} // namespace bonsai
} // namespace ocs
