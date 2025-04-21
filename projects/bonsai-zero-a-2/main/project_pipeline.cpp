/*
 * Copyright (c) 2025, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ocs_algo/mdns_ops.h"
#include "ocs_core/log.h"
#include "ocs_fmt/json/cjson_object_formatter.h"
#include "ocs_http/router.h"
#include "ocs_http/target_esp32/server.h"
#include "ocs_io/adc/target_esp32/line_fitting_converter.h"
#include "ocs_io/adc/target_esp32/oneshot_store.h"
#include "ocs_net/target_esp32/default_mdns_server.h"
#include "ocs_pipeline/jsonfmt/ap_network_formatter.h"
#include "ocs_pipeline/jsonfmt/soil_analog_sensor_formatter.h"
#include "ocs_pipeline/jsonfmt/sta_network_formatter.h"
#include "ocs_sensor/soil/soil_status_to_str.h"
#include "ocs_status/code_to_str.h"
#include "ocs_status/macros.h"

#include "main/project_pipeline.h"

namespace ocs {
namespace bonsai {

namespace {

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

    const auto mdns_instance_name = std::string("Bonsai Zero Analog 2 HTTP Service (")
        + system_pipeline_->get_device_info().get_fw_name() + ")";

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

    soil_sensor_config_0_.reset(new (std::nothrow) sensor::AnalogConfig(
        *analog_config_storage_, CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_0_ANALOG_VALUE_MIN,
        CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_0_ANALOG_VALUE_MAX, ADC_BITWIDTH_12,
        sensor::AnalogConfig::OversamplingMode::Mode_64, soil_sensor_id_0_));
    configASSERT(soil_sensor_config_0_);

    analog_config_store_->add(*soil_sensor_config_0_);

    soil_sensor_pipeline_0_.reset(new (std::nothrow) sensor::soil::AnalogSensorPipeline(
        system_pipeline_->get_clock(), *adc_store_, *adc_converter_,
        system_pipeline_->get_storage_builder(), *rt_delayer_,
        system_pipeline_->get_reboot_handler(), system_pipeline_->get_task_scheduler(),
        *soil_sensor_config_0_, soil_sensor_id_0_,
        sensor::soil::AnalogSensorPipeline::Params {
            .adc_channel = static_cast<io::adc::Channel>(
                CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_0_ANALOG_ADC_CHANNEL),
            .fsm_block =
                control::FsmBlockPipeline::Params {
                    .state_save_interval = core::Duration::hour * 2,
                    .state_interval_resolution = core::Duration::second,
                },
            .read_interval = core::Duration::second
                * CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_0_ANALOG_READ_INTERVAL,
        }));
    configASSERT(soil_sensor_pipeline_0_);

    soil_sensor_config_1_.reset(new (std::nothrow) sensor::AnalogConfig(
        *analog_config_storage_, CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_1_ANALOG_VALUE_MIN,
        CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_1_ANALOG_VALUE_MAX, ADC_BITWIDTH_12,
        sensor::AnalogConfig::OversamplingMode::Mode_64, soil_sensor_id_1_));
    configASSERT(soil_sensor_config_1_);

    analog_config_store_->add(*soil_sensor_config_1_);

    soil_sensor_pipeline_1_.reset(new (std::nothrow) sensor::soil::AnalogSensorPipeline(
        system_pipeline_->get_clock(), *adc_store_, *adc_converter_,
        system_pipeline_->get_storage_builder(), *rt_delayer_,
        system_pipeline_->get_reboot_handler(), system_pipeline_->get_task_scheduler(),
        *soil_sensor_config_1_, soil_sensor_id_1_,
        sensor::soil::AnalogSensorPipeline::Params {
            .adc_channel = static_cast<io::adc::Channel>(
                CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_1_ANALOG_ADC_CHANNEL),
            .fsm_block =
                control::FsmBlockPipeline::Params {
                    .state_save_interval = core::Duration::hour * 2,
                    .state_interval_resolution = core::Duration::second,
                },
            .read_interval = core::Duration::second
                * CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_1_ANALOG_READ_INTERVAL,
        }));
    configASSERT(soil_sensor_pipeline_1_);

    json_data_pipeline_->get_telemetry_formatter().add(*this);

    web_gui_pipeline_.reset(new (std::nothrow)
                                pipeline::httpserver::WebGuiPipeline(*http_router_));
    configASSERT(web_gui_pipeline_);
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

status::StatusCode ProjectPipeline::handle_suspend() {
    return mdns_server_->stop();
}

status::StatusCode ProjectPipeline::handle_resume() {
    return mdns_server_->start();
}

status::StatusCode ProjectPipeline::format(cJSON* json) {
    fmt::json::CjsonObjectFormatter formatter(json);

    const auto data0 = soil_sensor_pipeline_0_->get_sensor().get_data();

    if (!formatter.add_number_cs("s0_raw", data0.raw)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s0_voltage", data0.voltage)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s0_moisture", data0.moisture)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_string_ref_cs(
            "s0_prev_status", sensor::soil::soil_status_to_str(data0.prev_status))) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s0_prev_status_dur", data0.prev_status_duration)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_string_ref_cs(
            "s0_curr_status", sensor::soil::soil_status_to_str(data0.curr_status))) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s0_curr_status_dur", data0.curr_status_duration)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s0_write_count", data0.write_count)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s0_status_progress", data0.status_progress)) {
        return status::StatusCode::NoMem;
    }

    const auto data1 = soil_sensor_pipeline_1_->get_sensor().get_data();

    if (!formatter.add_number_cs("s1_raw", data1.raw)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s1_voltage", data1.voltage)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s1_moisture", data1.moisture)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_string_ref_cs(
            "s1_prev_status", sensor::soil::soil_status_to_str(data1.prev_status))) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s1_prev_status_dur", data1.prev_status_duration)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_string_ref_cs(
            "s1_curr_status", sensor::soil::soil_status_to_str(data1.curr_status))) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s1_curr_status_dur", data1.curr_status_duration)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s1_write_count", data1.write_count)) {
        return status::StatusCode::NoMem;
    }

    if (!formatter.add_number_cs("s1_status_progress", data1.status_progress)) {
        return status::StatusCode::NoMem;
    }

    return status::StatusCode::OK;
}

} // namespace bonsai
} // namespace ocs
