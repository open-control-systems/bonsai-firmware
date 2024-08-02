/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_log.h"

#include "ocs_status/code_to_str.h"

#include "scs/project_pipeline.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "project-pipeline";

} // namespace

ProjectPipeline::ProjectPipeline() {
    system_pipeline_.reset(new (std::nothrow) SystemPipeline());
    configASSERT(system_pipeline_);

    data_pipeline_.reset(new (std::nothrow) DataPipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_task_scheduler(),
        system_pipeline_->get_timer_store(), system_pipeline_->get_reboot_handler()));
    configASSERT(data_pipeline_);

    control_pipeline_.reset(new (std::nothrow) ControlPipeline(
        system_pipeline_->get_task_scheduler(), system_pipeline_->get_timer_store(),
        data_pipeline_->get_telemetry_writer()));
    configASSERT(control_pipeline_);

    http_pipeline_.reset(new (std::nothrow) HttpPipeline<256, 512>(
        system_pipeline_->get_reboot_task(), control_pipeline_->get_control_task(),
        data_pipeline_->get_telemetry_formatter(),
        data_pipeline_->get_registration_formatter()));
    configASSERT(http_pipeline_);
}

status::StatusCode ProjectPipeline::start() {
    auto code = http_pipeline_->start();
    if (code != status::StatusCode::OK) {
        ESP_LOGW(log_tag, "failed to start HTTP pipeline: %s", status::code_to_str(code));
    }

    code = control_pipeline_->start();
    if (code != status::StatusCode::OK) {
        return code;
    }

    code = system_pipeline_->start();
    if (code != status::StatusCode::OK) {
        return code;
    }

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs