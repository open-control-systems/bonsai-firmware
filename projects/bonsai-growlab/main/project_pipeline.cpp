/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ocs_algo/mdns_ops.h"
#include "ocs_core/log.h"
#include "ocs_io/adc/default_store.h"
#include "ocs_net/default_mdns_server.h"
#include "ocs_pipeline/jsonfmt/ap_network_formatter.h"
#include "ocs_pipeline/jsonfmt/sta_network_formatter.h"
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

    configASSERT(system_pipeline_->get_suspender().add(*this, "project_pipeline")
                 == status::StatusCode::OK);

    json_data_pipeline_.reset(new (std::nothrow) pipeline::jsonfmt::DataPipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_storage_builder(),
        system_pipeline_->get_task_scheduler(), system_pipeline_->get_reboot_handler(),
        system_pipeline_->get_device_info()));
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

    fanout_network_handler_.reset(new (std::nothrow) net::FanoutNetworkHandler());
    configASSERT(fanout_network_handler_);

    mdns_config_storage_ =
        system_pipeline_->get_storage_builder().make(mdns_config_storage_id_);
    configASSERT(mdns_config_storage_);

    mdns_config_.reset(new (std::nothrow) net::MdnsConfig(
        *mdns_config_storage_, system_pipeline_->get_device_info()));
    configASSERT(mdns_config_);

    http_mdns_service_.reset(new (std::nothrow) net::MdnsService(
        "Bonsai GrowLab HTTP Service", net::MdnsService::ServiceType::Http,
        net::MdnsService::Proto::Tcp, "local", mdns_config_->get_hostname(),
        CONFIG_OCS_HTTP_SERVER_PORT));
    configASSERT(http_mdns_service_);

    http_mdns_service_->add_txt_record("api", CONFIG_OCS_HTTP_SERVER_API_BASE_PATH);

    algo::MdnsOps::enable_autodiscovery(*http_mdns_service_,
                                        CONFIG_OCS_HTTP_SERVER_API_BASE_PATH);

    mdns_server_.reset(new (std::nothrow)
                           net::DefaultMdnsServer(mdns_config_->get_hostname()));
    configASSERT(mdns_server_);

    mdns_server_->add(*http_mdns_service_);

    http_pipeline_.reset(new (std::nothrow) pipeline::httpserver::HttpPipeline(
        system_pipeline_->get_reboot_task(), *fanout_network_handler_, *mdns_config_,
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
            .server =
                http::Server::Params {
                    .server_port = CONFIG_OCS_HTTP_SERVER_PORT,
                    .max_uri_handlers = CONFIG_OCS_HTTP_SERVER_MAX_URI_HANDLERS,
                },
        }));
    configASSERT(http_pipeline_);

    // Time valid since 2024/12/03.
    time_pipeline_.reset(new (std::nothrow) pipeline::httpserver::TimePipeline(
        http_pipeline_->get_server(), json_data_pipeline_->get_telemetry_formatter(),
        json_data_pipeline_->get_registration_formatter(), 1733215816));
    configASSERT(time_pipeline_);

    network_pipeline_.reset(new (std::nothrow) pipeline::basic::SelectNetworkPipeline(
        system_pipeline_->get_storage_builder(), *fanout_network_handler_,
        system_pipeline_->get_rebooter(), system_pipeline_->get_device_info()));
    configASSERT(network_pipeline_);

    if (auto network = network_pipeline_->get_ap_network(); network) {
        ap_network_formatter_.reset(new (std::nothrow)
                                        pipeline::jsonfmt::ApNetworkFormatter(*network));
        configASSERT(ap_network_formatter_);

        json_data_pipeline_->get_registration_formatter().add(*ap_network_formatter_);

        configASSERT(network_pipeline_->get_ap_config());

        ap_network_handler_.reset(
            new (std::nothrow) pipeline::httpserver::ApNetworkHandler(
                http_pipeline_->get_server(), *network_pipeline_->get_ap_config(),
                system_pipeline_->get_reboot_task()));
        configASSERT(ap_network_handler_);
    }

    if (auto network = network_pipeline_->get_sta_network(); network) {
        sta_network_formatter_.reset(
            new (std::nothrow) pipeline::jsonfmt::StaNetworkFormatter(*network));
        configASSERT(sta_network_formatter_);

        json_data_pipeline_->get_registration_formatter().add(*sta_network_formatter_);
    }

    sta_network_handler_.reset(new (std::nothrow) pipeline::httpserver::StaNetworkHandler(
        http_pipeline_->get_server(), network_pipeline_->get_sta_config(),
        system_pipeline_->get_reboot_task()));
    configASSERT(sta_network_handler_);

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
        system_pipeline_->get_func_scheduler(), system_pipeline_->get_storage_builder(),
        json_data_pipeline_->get_telemetry_formatter(), http_pipeline_->get_server(),
        core::Duration::second * CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_READ_INTERVAL));
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
        http_pipeline_->get_server()));
    configASSERT(ds18b20_pipeline_);
#endif // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE) ||
       // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)

    web_gui_pipeline_.reset(new (std::nothrow) pipeline::httpserver::WebGuiPipeline(
        http_pipeline_->get_server()));
    configASSERT(web_gui_pipeline_);
}

status::StatusCode ProjectPipeline::handle_suspend() {
    return mdns_server_->stop();
}

status::StatusCode ProjectPipeline::handle_resume() {
    return mdns_server_->start();
}

status::StatusCode ProjectPipeline::start() {
    auto code = network_pipeline_->get_runner().start();
    if (code == status::StatusCode::OK) {
        code = mdns_server_->start();
        if (code != status::StatusCode::OK) {
            ocs_logw(log_tag, "failed to start mDNS server: %s",
                     status::code_to_str(code));
        }
    } else {
        ocs_logw(log_tag, "failed to start network: %s", status::code_to_str(code));
    }

    OCS_STATUS_RETURN_ON_ERROR(system_pipeline_->start());

    return status::StatusCode::OK;
}

} // namespace bonsai
} // namespace ocs
