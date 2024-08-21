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
namespace scs {

namespace {

const char* log_tag = "project-pipeline";

} // namespace

ProjectPipeline::ProjectPipeline() {
    system_pipeline_.reset(new (std::nothrow) iot::SystemPipeline());
    configASSERT(system_pipeline_);

    json_data_pipeline_.reset(new (std::nothrow) iot::JsonDataPipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_storage_builder(),
        system_pipeline_->get_task_scheduler(), system_pipeline_->get_timer_store(),
        system_pipeline_->get_reboot_handler()));
    configASSERT(json_data_pipeline_);

    control_pipeline_.reset(new (std::nothrow) ControlPipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_storage_builder(),
        system_pipeline_->get_reboot_handler(), system_pipeline_->get_task_scheduler(),
        system_pipeline_->get_timer_store(), json_data_pipeline_->get_counter_holder(),
        json_data_pipeline_->get_telemetry_formatter()));
    configASSERT(control_pipeline_);

    console_pipeline_.reset(new (std::nothrow) iot::ConsoleJsonPipeline(
        system_pipeline_->get_task_scheduler(), system_pipeline_->get_timer_store(),
        json_data_pipeline_->get_telemetry_formatter(),
        json_data_pipeline_->get_registration_formatter(),
        iot::ConsoleJsonPipeline::Params {
            .telemetry =
                iot::ConsoleJsonPipeline::DataParams {
                    .interval = core::Second * 10,
                    .buffer_size = 512,
                },
            .registration =
                iot::ConsoleJsonPipeline::DataParams {
                    .interval = core::Second * 20,
                    .buffer_size = 256,
                },
        }));
    configASSERT(console_pipeline_);

    http_pipeline_.reset(new (std::nothrow) iot::HttpPipeline(
        system_pipeline_->get_reboot_task(), control_pipeline_->get_control_task(),
        json_data_pipeline_->get_telemetry_formatter(),
        json_data_pipeline_->get_registration_formatter(),
        iot::HttpPipeline::Params {
            .telemetry =
                iot::HttpPipeline::DataParams {
                    .buffer_size = 512,
                },
            .registration =
                iot::HttpPipeline::DataParams {
                    .buffer_size = 256,
                },
            .commands =
                iot::HttpPipeline::DataParams {
                    .buffer_size = 256,
                },
        }));
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

} // namespace scs
} // namespace ocs
