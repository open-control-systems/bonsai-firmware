/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ocs_core/log.h"
#include "ocs_status/code_to_str.h"
#include "ocs_status/macros.h"

#include "project_pipeline.h"

namespace ocs {
namespace bonsai {

namespace {

const char* log_tag = "project-pipeline";

} // namespace

ProjectPipeline::ProjectPipeline() {
    system_pipeline_.reset(new (std::nothrow)
                               pipeline::SystemPipeline(pipeline::SystemPipeline::Params {
                                   .task_scheduler =
                                       pipeline::SystemPipeline::Params::TaskScheduler {
                                           .delay = pdMS_TO_TICKS(200),
                                       },
                               }));
    configASSERT(system_pipeline_);

    json_data_pipeline_.reset(new (std::nothrow) pipeline::JsonDataPipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_storage_builder(),
        system_pipeline_->get_task_scheduler(), system_pipeline_->get_reboot_handler(),
        pipeline::RegistrationJsonFormatter::Params {
            .fw_version = CONFIG_OCS_CORE_FW_VERSION,
            .fw_name = CONFIG_OCS_CORE_FW_NAME,
        }));
    configASSERT(json_data_pipeline_);

#ifdef CONFIG_OCS_PIPELINE_CONSOLE_PIPELINE_ENABLE
    console_pipeline_.reset(new (std::nothrow) pipeline::ConsoleJsonPipeline(
        system_pipeline_->get_task_scheduler(),
        json_data_pipeline_->get_telemetry_formatter(),
        json_data_pipeline_->get_registration_formatter(),
        pipeline::ConsoleJsonPipeline::Params {
            .telemetry =
                pipeline::ConsoleJsonPipeline::DataParams {
                    .interval = core::Second
                        * CONFIG_OCS_PIPELINE_CONSOLE_PIPELINE_TELEMETRY_INTERVAL,
                    .buffer_size =
                        CONFIG_OCS_PIPELINE_CONSOLE_PIPELINE_TELEMETRY_BUFFER_SIZE,
                },
            .registration =
                pipeline::ConsoleJsonPipeline::DataParams {
                    .interval = core::Second
                        * CONFIG_OCS_PIPELINE_CONSOLE_PIPELINE_REGISTRATION_INTERVAL,
                    .buffer_size =
                        CONFIG_OCS_PIPELINE_CONSOLE_PIPELINE_REGISTRATION_BUFFER_SIZE,
                },
        }));
    configASSERT(console_pipeline_);
#endif // CONFIG_OCS_PIPELINE_CONSOLE_PIPELINE_ENABLE

    http_pipeline_.reset(new (std::nothrow) pipeline::HttpPipeline(
        system_pipeline_->get_reboot_task(), system_pipeline_->get_suspender(),
        json_data_pipeline_->get_telemetry_formatter(),
        json_data_pipeline_->get_registration_formatter(),
        pipeline::HttpPipeline::Params {
            .telemetry =
                pipeline::HttpPipeline::DataParams {
                    .buffer_size =
                        CONFIG_OCS_PIPELINE_HTTP_PIPELINE_TELEMETRY_BUFFER_SIZE,
                },
            .registration =
                pipeline::HttpPipeline::DataParams {
                    .buffer_size =
                        CONFIG_OCS_PIPELINE_HTTP_PIPELINE_REGISTRATION_BUFFER_SIZE,
                },
        }));
    configASSERT(http_pipeline_);

    control_pipeline_.reset(new (std::nothrow) ControlPipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_storage_builder(),
        system_pipeline_->get_reboot_handler(), system_pipeline_->get_task_scheduler(),
        json_data_pipeline_->get_counter_holder(),
        json_data_pipeline_->get_telemetry_formatter()));
    configASSERT(control_pipeline_);

#if defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE)               \
    || defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)
    ds18b20_pipeline_.reset(new (std::nothrow) DS18B20Pipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_storage_builder(),
        system_pipeline_->get_task_scheduler(),
        json_data_pipeline_->get_telemetry_formatter()));
    configASSERT(ds18b20_pipeline_);

    ds18b20_sensor_http_handler_.reset(new (std::nothrow) pipeline::ds18b20::HttpHandler(
        http_pipeline_->get_server_pipeline().server(),
        http_pipeline_->get_server_pipeline().mdns(), system_pipeline_->get_suspender(),
        ds18b20_pipeline_->get_store()));
    configASSERT(ds18b20_sensor_http_handler_);
#endif // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE) ||
       // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)
}

status::StatusCode ProjectPipeline::start() {
    const auto code = http_pipeline_->start();
    if (code != status::StatusCode::OK) {
        ocs_logw(log_tag, "failed to start HTTP pipeline: %s", status::code_to_str(code));
    }

    OCS_STATUS_RETURN_ON_ERROR(system_pipeline_->start());

    return status::StatusCode::OK;
}

} // namespace bonsai
} // namespace ocs
