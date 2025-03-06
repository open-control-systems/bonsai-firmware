/*
 * Copyright (c) 2025, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ocs_algo/bit_ops.h"
#include "ocs_algo/mdns_ops.h"
#include "ocs_core/log.h"
#include "ocs_http/router.h"
#include "ocs_http/target_esp32/server.h"
#include "ocs_io/adc/target_esp32/line_fitting_converter.h"
#include "ocs_io/adc/target_esp32/oneshot_store.h"
#include "ocs_net/target_esp32/default_mdns_server.h"
#include "ocs_pipeline/jsonfmt/ap_network_formatter.h"
#include "ocs_pipeline/jsonfmt/soil_analog_sensor_formatter.h"
#include "ocs_pipeline/jsonfmt/sta_network_formatter.h"
#include "ocs_status/code_to_str.h"
#include "ocs_status/macros.h"

#include "main/project_pipeline.h"

namespace ocs {
namespace bonsai {

namespace {

void configure_relay_gpio(int gpio) {
    gpio_config_t config;
    memset(&config, 0, sizeof(config));

    // disable interrupt
    config.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    config.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,
    config.pin_bit_mask = algo::BitOps::mask(gpio);
    // enable pull-down mode
    config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    // disable pull-up mode
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    // configure GPIO with the given settings
    gpio_config(&config);
}

const char* log_tag = "project_pipeline";

} // namespace

ProjectPipeline::ProjectPipeline() {
    rt_delayer_ = system::PlatformBuilder::make_rt_delayer();
    configASSERT(rt_delayer_);

    fanout_suspender_.reset(new (std::nothrow) system::FanoutSuspender());
    configASSERT(fanout_suspender_);

    system_pipeline_.reset(new (std::nothrow) pipeline::basic::SystemPipeline(
        pipeline::basic::SystemPipeline::Params {
            .task_scheduler =
                pipeline::basic::SystemPipeline::Params::TaskScheduler {
                    .delay = pdMS_TO_TICKS(200),
                },
        }));
    configASSERT(system_pipeline_);

    configASSERT(fanout_suspender_->add(*this, "project_pipeline")
                 == status::StatusCode::OK);

    json_data_pipeline_.reset(new (std::nothrow) pipeline::jsonfmt::DataPipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_storage_builder(),
        system_pipeline_->get_task_scheduler(), system_pipeline_->get_reboot_handler(),
        system_pipeline_->get_device_info()));
    configASSERT(json_data_pipeline_);

    fanout_network_handler_.reset(new (std::nothrow) net::FanoutNetworkHandler());
    configASSERT(fanout_network_handler_);

    mdns_config_storage_ =
        system_pipeline_->get_storage_builder().make(mdns_config_storage_id_);
    configASSERT(mdns_config_storage_);

    mdns_config_.reset(new (std::nothrow) net::MdnsConfig(
        *mdns_config_storage_, system_pipeline_->get_device_info()));
    configASSERT(mdns_config_);

    const auto mdns_instance_name =
        std::string("Bonsai Zero Analog Relay 1X HTTP Service (")
        + mdns_config_->get_hostname() + ")";

    http_mdns_service_.reset(new (std::nothrow) net::MdnsService(
        mdns_instance_name.c_str(), net::MdnsService::ServiceType::Http,
        net::MdnsService::Proto::Tcp, "local", mdns_config_->get_hostname(),
        CONFIG_OCS_HTTP_SERVER_PORT));
    configASSERT(http_mdns_service_);

    http_mdns_service_->add_txt_record("api", CONFIG_OCS_HTTP_SERVER_API_BASE_PATH);

    algo::MdnsOps::enable_autodiscovery(*http_mdns_service_,
                                        system_pipeline_->get_device_info().get_fw_name(),
                                        CONFIG_OCS_HTTP_SERVER_API_BASE_PATH);

    mdns_server_.reset(new (std::nothrow)
                           net::DefaultMdnsServer(mdns_config_->get_hostname()));
    configASSERT(mdns_server_);

    mdns_server_->add(*http_mdns_service_);

    http_router_.reset(new (std::nothrow) http::Router());
    configASSERT(http_router_);

    http_server_.reset(new (std::nothrow) http::Server(
        *http_router_,
        http::Server::Params {
            .server_port = CONFIG_OCS_HTTP_SERVER_PORT,
            .max_uri_handlers = CONFIG_OCS_HTTP_SERVER_MAX_URI_HANDLERS,
        }));
    configASSERT(http_server_);

    http_pipeline_.reset(new (std::nothrow) pipeline::httpserver::HttpPipeline(
        system_pipeline_->get_reboot_task(), *fanout_network_handler_, *mdns_config_,
        *http_server_, *http_router_, json_data_pipeline_->get_telemetry_formatter(),
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

    // Time valid since 2024/12/03.
    time_pipeline_.reset(new (std::nothrow) pipeline::httpserver::TimePipeline(
        *http_router_, json_data_pipeline_->get_telemetry_formatter(),
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
                *http_router_, *network_pipeline_->get_ap_config(),
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
        *http_router_, network_pipeline_->get_sta_config(),
        system_pipeline_->get_reboot_task()));
    configASSERT(sta_network_handler_);

    adc_store_.reset(new (std::nothrow) io::adc::OneshotStore(ADC_UNIT_1, ADC_ATTEN_DB_12,
                                                              ADC_BITWIDTH_12));
    configASSERT(adc_store_);

    adc_converter_.reset(new (std::nothrow) io::adc::LineFittingConverter(
        ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_BITWIDTH_12));
    configASSERT(adc_converter_);

    analog_config_storage_ =
        system_pipeline_->get_storage_builder().make(analog_config_storage_id_);
    configASSERT(analog_config_storage_);

    analog_config_store_.reset(new (std::nothrow) sensor::AnalogConfigStore());
    configASSERT(analog_config_store_);

    analog_config_store_handler_.reset(new (std::nothrow)
                                           pipeline::httpserver::AnalogConfigStoreHandler(
                                               system_pipeline_->get_func_scheduler(),
                                               *http_router_, *analog_config_store_));
    configASSERT(analog_config_store_handler_);

    soil_relay_sensor_config_.reset(new (std::nothrow) sensor::AnalogConfig(
        *analog_config_storage_,
        CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_VALUE_MIN,
        CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_VALUE_MAX, ADC_BITWIDTH_12,
        sensor::AnalogConfig::OversamplingMode::Mode_64, soil_relay_sensor_id_));
    configASSERT(soil_relay_sensor_config_);

    analog_config_store_->add(*soil_relay_sensor_config_);

    soil_relay_sensor_pipeline_.reset(
        new (std::nothrow) sensor::soil::AnalogRelaySensorPipeline(
            system_pipeline_->get_clock(), *adc_store_, *adc_converter_,
            system_pipeline_->get_storage_builder(), *rt_delayer_,
            system_pipeline_->get_reboot_handler(),
            system_pipeline_->get_task_scheduler(), *soil_relay_sensor_config_,
            soil_relay_sensor_id_,
            sensor::soil::AnalogRelaySensorPipeline::Params {
                .adc_channel = static_cast<io::adc::Channel>(
                    CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ADC_CHANNEL),
                .fsm_block =
                    control::FsmBlockPipeline::Params {
                        .state_save_interval = core::Duration::hour * 2,
                        .state_interval_resolution = core::Duration::second,
                    },
                .read_interval = core::Duration::second
                    * CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_READ_INTERVAL,
                .relay_gpio = static_cast<io::gpio::Gpio>(
                    CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_GPIO),
                .power_on_delay_interval =
                    (1000
                     * CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_POWER_ON_DELAY_INTERVAL)
                    / portTICK_PERIOD_MS,
            }));
    configASSERT(soil_relay_sensor_pipeline_);

    soil_relay_sensor_json_formatter_.reset(
        new (std::nothrow) pipeline::jsonfmt::SoilAnalogSensorFormatter(
            soil_relay_sensor_pipeline_->get_sensor()));
    configASSERT(soil_relay_sensor_json_formatter_);

    json_data_pipeline_->get_telemetry_formatter().add(
        *soil_relay_sensor_json_formatter_);

    configure_relay_gpio(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_GPIO);

    web_gui_pipeline_.reset(new (std::nothrow)
                                pipeline::httpserver::WebGuiPipeline(*http_router_));
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
