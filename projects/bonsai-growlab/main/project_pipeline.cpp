/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ocs_core/log.h"
#include "ocs_io/adc/default_store.h"
#include "ocs_status/code_to_str.h"
#include "ocs_status/macros.h"

#include "main/project_pipeline.h"

namespace ocs {
namespace bonsai {

namespace {

const char* log_tag = "project_pipeline";

} // namespace

ProjectPipeline::ProjectPipeline() {
    system_pipeline_.reset(new (std::nothrow) pipeline::basic::SystemPipeline(
        pipeline::basic::SystemPipeline::Params {
            .task_scheduler =
                pipeline::basic::SystemPipeline::Params::TaskScheduler {
                    .delay = pdMS_TO_TICKS(200),
                },
        }));
    configASSERT(system_pipeline_);

    json_data_pipeline_.reset(new (std::nothrow) pipeline::jsonfmt::DataPipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_storage_builder(),
        system_pipeline_->get_task_scheduler(), system_pipeline_->get_reboot_handler(),
        pipeline::jsonfmt::RegistrationFormatter::Params {
            .fw_version = CONFIG_OCS_CORE_FW_VERSION,
            .fw_name = CONFIG_OCS_CORE_FW_NAME,
        }));
    configASSERT(json_data_pipeline_);

#ifdef CONFIG_BONSAI_FIRMWARE_CONSOLE_ENABLE
    console_pipeline_.reset(new (std::nothrow) pipeline::jsonfmt::ConsolePipeline(
        system_pipeline_->get_task_scheduler(),
        json_data_pipeline_->get_telemetry_formatter(),
        json_data_pipeline_->get_registration_formatter(),
        pipeline::jsonfmt::ConsolePipeline::Params {
            .telemetry =
                pipeline::jsonfmt::ConsolePipeline::DataParams {
                    .interval = core::Duration::second
                        * CONFIG_BONSAI_FIRMWARE_CONSOLE_TELEMETRY_INTERVAL,
                    .buffer_size = CONFIG_BONSAI_FIRMWARE_CONSOLE_TELEMETRY_BUFFER_SIZE,
                },
            .registration =
                pipeline::jsonfmt::ConsolePipeline::DataParams {
                    .interval = core::Duration::second
                        * CONFIG_BONSAI_FIRMWARE_CONSOLE_REGISTRATION_INTERVAL,
                    .buffer_size =
                        CONFIG_BONSAI_FIRMWARE_CONSOLE_REGISTRATION_BUFFER_SIZE,
                },
        }));
    configASSERT(console_pipeline_);
#endif // CONFIG_BONSAI_FIRMWARE_CONSOLE_ENABLE

    http_pipeline_.reset(new (std::nothrow) pipeline::httpserver::HttpPipeline(
        system_pipeline_->get_reboot_task(), system_pipeline_->get_suspender(),
        json_data_pipeline_->get_telemetry_formatter(),
        json_data_pipeline_->get_registration_formatter(),
        pipeline::httpserver::HttpPipeline::Params {
            .telemetry =
                pipeline::httpserver::HttpPipeline::DataParams {
                    .buffer_size = CONFIG_BONSAI_FIRMWARE_HTTP_TELEMETRY_BUFFER_SIZE,
                },
            .registration =
                pipeline::httpserver::HttpPipeline::DataParams {
                    .buffer_size = CONFIG_BONSAI_FIRMWARE_HTTP_REGISTRATION_BUFFER_SIZE,
                },
        }));
    configASSERT(http_pipeline_);

    adc_store_.reset(new (std::nothrow)
                         io::adc::DefaultStore(io::adc::DefaultStore::Params {
                             .unit = ADC_UNIT_1,
                             .atten = ADC_ATTEN_DB_12,
                             .bitwidth = ADC_BITWIDTH_10,
                         }));
    configASSERT(adc_store_);

    i2c_master_store_pipeline_.reset(new (
        std::nothrow) io::i2c::MasterStorePipeline(io::i2c::IStore::Params {
        .sda = static_cast<io::gpio::Gpio>(CONFIG_BONSAI_FIRMWARE_I2C_MASTER_SDA_GPIO),
        .scl = static_cast<io::gpio::Gpio>(CONFIG_BONSAI_FIRMWARE_I2C_MASTER_SCL_GPIO),
    }));

    control_pipeline_.reset(new (std::nothrow) ControlPipeline(
        *adc_store_, system_pipeline_->get_clock(),
        system_pipeline_->get_storage_builder(), system_pipeline_->get_reboot_handler(),
        system_pipeline_->get_task_scheduler(),
        json_data_pipeline_->get_telemetry_formatter()));
    configASSERT(control_pipeline_);

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
    sht41_pipeline_.reset(new (std::nothrow) SHT41Pipeline(
        i2c_master_store_pipeline_->get_store(), system_pipeline_->get_task_scheduler(),
        json_data_pipeline_->get_telemetry_formatter()));
    configASSERT(sht41_pipeline_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#if defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE)                            \
    || defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE)
    soil_pipeline_.reset(new (std::nothrow) SoilPipeline(
        *adc_store_, system_pipeline_->get_clock(),
        system_pipeline_->get_storage_builder(), system_pipeline_->get_reboot_handler(),
        system_pipeline_->get_task_scheduler(),
        json_data_pipeline_->get_telemetry_formatter()));
    configASSERT(soil_pipeline_);
#endif // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE) ||
       // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE)

#if defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE)               \
    || defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)
    ds18b20_pipeline_.reset(new (std::nothrow) DS18B20Pipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_storage_builder(),
        system_pipeline_->get_task_scheduler(),
        json_data_pipeline_->get_telemetry_formatter(), system_pipeline_->get_suspender(),
        http_pipeline_->get_server_pipeline().server(),
        http_pipeline_->get_server_pipeline().mdns()));
    configASSERT(ds18b20_pipeline_);
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
