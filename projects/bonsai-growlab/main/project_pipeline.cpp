/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ocs_algo/bit_ops.h"
#include "ocs_algo/mdns_ops.h"
#include "ocs_core/log.h"
#include "ocs_io/adc/target_esp32/line_fitting_converter.h"
#include "ocs_io/adc/target_esp32/oneshot_store.h"
#include "ocs_io/spi/master_store.h"
#include "ocs_io/spi/types.h"
#include "ocs_net/default_mdns_server.h"
#include "ocs_pipeline/jsonfmt/ap_network_formatter.h"
#include "ocs_pipeline/jsonfmt/sta_network_formatter.h"
#include "ocs_status/code_to_str.h"
#include "ocs_status/macros.h"

#include "main/project_pipeline.h"

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ANALOG_ENABLE
#include "ocs_pipeline/jsonfmt/ldr_analog_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ANALOG_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ANALOG_ENABLE
#include "ocs_pipeline/jsonfmt/ldr_analog_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ANALOG_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE
#include "ocs_pipeline/jsonfmt/soil_analog_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE
#include "ocs_pipeline/jsonfmt/soil_analog_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
#include "ocs_pipeline/jsonfmt/bme280_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE

namespace ocs {
namespace bonsai {

namespace {

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE
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
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE

const char* log_tag = "project_pipeline";

} // namespace

ProjectPipeline::ProjectPipeline() {
    delayer_ = system::make_delayer(system::DelayerStrategy::Default);
    configASSERT(delayer_);

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

    const auto mdns_instance_name =
        std::string("Bonsai GrowLab HTTP Service (") + mdns_config_->get_hostname() + ")";

    http_mdns_service_.reset(new (std::nothrow) net::MdnsService(
        mdns_instance_name.c_str(), net::MdnsService::ServiceType::Http,
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

    adc_store_.reset(new (std::nothrow) io::adc::OneshotStore(ADC_UNIT_1, ADC_ATTEN_DB_12,
                                                              ADC_BITWIDTH_12));
    configASSERT(adc_store_);

    adc_converter_.reset(new (std::nothrow) io::adc::LineFittingConverter(
        ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_BITWIDTH_12));
    configASSERT(adc_converter_);

    i2c_master_store_pipeline_.reset(new (
        std::nothrow) io::i2c::MasterStorePipeline(io::i2c::IStore::Params {
        .sda = static_cast<io::gpio::Gpio>(CONFIG_BONSAI_FIRMWARE_I2C_MASTER_SDA_GPIO),
        .scl = static_cast<io::gpio::Gpio>(CONFIG_BONSAI_FIRMWARE_I2C_MASTER_SCL_GPIO),
    }));

    spi_master_store_.reset(new (
        std::nothrow) io::spi::MasterStore(io::spi::MasterStore::Params {
        .mosi = static_cast<io::gpio::Gpio>(CONFIG_BONSAI_FIRMWARE_SPI_MASTER_MOSI_GPIO),
        .miso = static_cast<io::gpio::Gpio>(CONFIG_BONSAI_FIRMWARE_SPI_MASTER_MISO_GPIO),
        .sclk = static_cast<io::gpio::Gpio>(CONFIG_BONSAI_FIRMWARE_SPI_MASTER_SCLK_GPIO),
        .max_transfer_size = static_cast<io::gpio::Gpio>(
            CONFIG_BONSAI_FIRMWARE_SPI_MASTER_MAX_TRANSFER_SIZE),
        .host_id = io::spi::VSPI,
    }));
    configASSERT(spi_master_store_);

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_SPI_ENABLE
    bme280_spi_sensor_pipeline_.reset(
        new (std::nothrow) sensor::bme280::SpiSensorPipeline(
            system_pipeline_->get_task_scheduler(), *spi_master_store_,
            sensor::bme280::SpiSensorPipeline::Params {
                .read_interval = CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_READ_INTERVAL
                    * core::Duration::second,
                .cs_gpio = static_cast<io::gpio::Gpio>(
                    CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_CS_GPIO),
                .sensor =
                    sensor::bme280::Sensor::Params {
                        .operation_mode = sensor::bme280::Sensor::OperationMode::Forced,
                        .pressure_oversampling =
                            sensor::bme280::Sensor::OversamplingMode::Mode_1,
                        .temperature_oversampling =
                            sensor::bme280::Sensor::OversamplingMode::Mode_1,
                        .humidity_oversampling =
                            sensor::bme280::Sensor::OversamplingMode::Mode_1,
                        .pressure_resolution = 2,
                        .pressure_decimal_places = 2,
                        .humidity_decimal_places = 2,
                    },
            }));
    configASSERT(bme280_spi_sensor_pipeline_);

    bme280_sensor_json_formatter_.reset(
        new (std::nothrow) pipeline::jsonfmt::BME280SensorFormatter(
            bme280_spi_sensor_pipeline_->get_sensor()));
    configASSERT(bme280_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_SPI_ENABLE

    json_data_pipeline_->get_telemetry_formatter().add(*bme280_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE

    analog_config_storage_ =
        system_pipeline_->get_storage_builder().make(analog_config_storage_id_);
    configASSERT(analog_config_storage_);

    analog_config_store_.reset(new (std::nothrow) sensor::AnalogConfigStore());
    configASSERT(analog_config_store_);

    analog_config_store_handler_.reset(
        new (std::nothrow) pipeline::httpserver::AnalogConfigStoreHandler(
            system_pipeline_->get_func_scheduler(), http_pipeline_->get_server(),
            *analog_config_store_));
    configASSERT(analog_config_store_handler_);

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ANALOG_ENABLE
    ldr_sensor_config_.reset(new (std::nothrow) sensor::AnalogConfig(
        *analog_config_storage_, CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ANALOG_VALUE_MIN,
        CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ANALOG_VALUE_MAX, ADC_BITWIDTH_12,
        sensor::AnalogConfig::OversamplingMode::Mode_64, ldr_sensor_id_));
    configASSERT(ldr_sensor_config_);

    analog_config_store_->add(*ldr_sensor_config_);

    ldr_sensor_pipeline_.reset(new (std::nothrow) sensor::ldr::AnalogSensorPipeline(
        *delayer_, *adc_store_, *adc_converter_, system_pipeline_->get_task_scheduler(),
        *ldr_sensor_config_, ldr_sensor_id_,
        sensor::ldr::AnalogSensorPipeline::Params {
            .adc_channel = static_cast<io::adc::Channel>(
                CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ANALOG_ADC_CHANNEL),
            .read_interval = core::Duration::second
                * CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ANALOG_READ_INTERVAL,
        }));
    configASSERT(ldr_sensor_pipeline_);

    ldr_sensor_json_formatter_.reset(new (std::nothrow)
                                         pipeline::jsonfmt::LdrAnalogSensorFormatter(
                                             ldr_sensor_pipeline_->get_sensor()));
    configASSERT(ldr_sensor_json_formatter_);

    json_data_pipeline_->get_telemetry_formatter().add(*ldr_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ANALOG_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE
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
            system_pipeline_->get_storage_builder(), *delayer_,
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
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE
    soil_sensor_config_.reset(new (std::nothrow) sensor::AnalogConfig(
        *analog_config_storage_, CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_VALUE_MIN,
        CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_VALUE_MAX, ADC_BITWIDTH_12,
        sensor::AnalogConfig::OversamplingMode::Mode_64, soil_sensor_id_));
    configASSERT(soil_sensor_config_);

    analog_config_store_->add(*soil_sensor_config_);

    soil_sensor_pipeline_.reset(new (std::nothrow) sensor::soil::AnalogSensorPipeline(
        system_pipeline_->get_clock(), *adc_store_, *adc_converter_,
        system_pipeline_->get_storage_builder(), *delayer_,
        system_pipeline_->get_reboot_handler(), system_pipeline_->get_task_scheduler(),
        *soil_sensor_config_, soil_sensor_id_,
        sensor::soil::AnalogSensorPipeline::Params {
            .adc_channel = static_cast<io::adc::Channel>(
                CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ADC_CHANNEL),
            .fsm_block =
                control::FsmBlockPipeline::Params {
                    .state_save_interval = core::Duration::hour * 2,
                    .state_interval_resolution = core::Duration::second,
                },
            .read_interval = core::Duration::second
                * CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_READ_INTERVAL,
        }));
    configASSERT(soil_sensor_pipeline_);

    soil_sensor_json_formatter_.reset(new (std::nothrow)
                                          pipeline::jsonfmt::SoilAnalogSensorFormatter(
                                              soil_sensor_pipeline_->get_sensor()));
    configASSERT(soil_sensor_json_formatter_);

    json_data_pipeline_->get_telemetry_formatter().add(*soil_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
    sht41_pipeline_.reset(new (std::nothrow) SHT41Pipeline(
        i2c_master_store_pipeline_->get_store(), system_pipeline_->get_task_scheduler(),
        system_pipeline_->get_func_scheduler(), system_pipeline_->get_storage_builder(),
        json_data_pipeline_->get_telemetry_formatter(), http_pipeline_->get_server(),
        core::Duration::second * CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_READ_INTERVAL));
    configASSERT(sht41_pipeline_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#if defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE)               \
    || defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)
    ds18b20_pipeline_.reset(new (std::nothrow) DS18B20Pipeline(
        system_pipeline_->get_clock(), system_pipeline_->get_storage_builder(),
        system_pipeline_->get_task_scheduler(),
        json_data_pipeline_->get_telemetry_formatter(), *delayer_, *fanout_suspender_,
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
