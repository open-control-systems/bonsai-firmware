/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_log.h"

#include "adc_reader.h"
#include "console_telemetry_writer.h"
#include "control_pipeline.h"
#include "ocs_status/code_to_str.h"
#include "ocs_system/default_clock.h"
#include "ocs_system/default_rebooter.h"
#include "ocs_system/delay_rebooter.h"
#include "yl69_moisture_reader.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "control-pipeline";

} // namespace

ControlPipeline::ControlPipeline() {
    adc_reader_.reset(new (std::nothrow) ADCReader(ADCReader::Params {
        .channel = static_cast<adc_channel_t>(CONFIG_SMC_SENSOR_ADC_CHANNEL),
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    }));

    moisture_reader_.reset(
        new (std::nothrow) YL69MoistureReader(CONFIG_SMC_SENSOR_THRESHOLD, *adc_reader_));

    flash_initializer_.reset(new (std::nothrow) storage::FlashInitializer());

    wifi_network_.reset(new (std::nothrow) net::WiFiNetwork(net::WiFiNetwork::Params {
        .max_retry_count = CONFIG_OCS_NETWORK_WIFI_STA_RETRY_COUNT,
        .ssid = CONFIG_OCS_NETWORK_WIFI_STA_SSID,
        .password = CONFIG_OCS_NETWORK_WIFI_STA_PASSWORD,
    }));
    wifi_network_->add(*this);

    http_server_.reset(new (std::nothrow) net::HTTPServer(net::HTTPServer::Params {
        .server_port = CONFIG_OCS_NETWORK_HTTP_SERVER_PORT,
    }));

    mdns_provider_.reset(new (std::nothrow) net::MDNSProvider(net::MDNSProvider::Params {
        .hostname = CONFIG_OCS_NETWORK_MDNS_HOSTNAME,
        .instance_name = CONFIG_OCS_NETWORK_MDNS_INSTANCE_NAME,
    }));

    default_clock_.reset(new (std::nothrow) system::DefaultClock());

    fanout_reboot_handler_.reset(new (std::nothrow) system::FanoutRebootHandler());

    default_rebooter_.reset(new (std::nothrow)
                                system::DefaultRebooter(*fanout_reboot_handler_));

    delay_rebooter_.reset(
        new (std::nothrow) system::DelayRebooter(pdMS_TO_TICKS(500), *default_rebooter_));

    fanout_telemetry_writer_.reset(new (std::nothrow) FanoutTelemetryWriter());

    telemetry_holder_.reset(new (std::nothrow) TelemetryHolder());
    fanout_telemetry_writer_->add(*telemetry_holder_);

    telemetry_formatter_.reset(new (std::nothrow) TelemetryFormatter());
    telemetry_formatter_->fanout().add(*telemetry_holder_);

    http_telemetry_handler_.reset(new (std::nothrow) HTTPTelemetryHandler(
        *http_server_, *telemetry_formatter_, "/telemetry", "http-telemetry-handler"));

    console_telemetry_writer_.reset(new (std::nothrow)
                                        ConsoleTelemetryWriter(*telemetry_formatter_));
    fanout_telemetry_writer_->add(*console_telemetry_writer_);

    gpio_config_.reset(new (std::nothrow) GPIOConfig());

    soil_moisture_monitor_.reset(new (std::nothrow) SoilMoistureMonitor(
        SoilMoistureMonitor::Params {
            .power_on_delay_interval =
                (1000 * CONFIG_SMC_POWER_ON_DELAY_INTERVAL) / portTICK_PERIOD_MS,
            .read_interval = (1000 * CONFIG_SMC_READ_INTERVAL) / portTICK_PERIOD_MS,
            .relay_gpio = static_cast<gpio_num_t>(CONFIG_SMC_RELAY_GPIO),
        },
        *moisture_reader_, *fanout_telemetry_writer_));

    http_command_handler_.reset(new (std::nothrow) HTTPCommandHandler(
        *delay_rebooter_, *http_server_, *soil_moisture_monitor_));

    registration_formatter_.reset(new (std::nothrow)
                                      RegistrationFormatter(*wifi_network_));

    http_registration_handler_.reset(new (std::nothrow) HTTPRegistrationHandler(
        *http_server_, *registration_formatter_, "/registration",
        "http-registration-handler"));

    storage_builder_.reset(new (std::nothrow) storage::StorageBuilder());
    counter_json_formatter_.reset(new (std::nothrow) iot::CounterJSONFormatter());

    system_counter_pipeline_.reset(new (std::nothrow) iot::SystemCounterPipeline(
        *default_clock_, *storage_builder_, *fanout_reboot_handler_,
        *counter_json_formatter_));

    telemetry_formatter_->fanout().add(*counter_json_formatter_);
}

void ControlPipeline::handle_connected() {
    http_server_->start();
}

void ControlPipeline::handle_disconnected() {
    http_server_->stop();
}

void ControlPipeline::start() {
    if (try_start_wifi_()) {
        try_start_mdns_();
    }

    soil_moisture_monitor_->start();
}

bool ControlPipeline::try_start_wifi_() {
    auto code = wifi_network_->start();
    if (code != status::StatusCode::OK) {
        return false;
    }

    code = wifi_network_->wait();
    if (code != status::StatusCode::OK) {
        ESP_LOGE(log_tag, "failed to start the WiFi connection process: code=%s",
                 status::code_to_str(code));

        code = wifi_network_->stop();
        if (code != status::StatusCode::OK) {
            ESP_LOGE(log_tag, "failed to stop the WiFi connection process: code=%s",
                     status::code_to_str(code));
        }

        wifi_network_ = nullptr;

        return false;
    }

    return true;
}

void ControlPipeline::try_start_mdns_() {
    auto code = mdns_provider_->start();
    if (code == status::StatusCode::OK) {
        code = mdns_provider_->add_service("_http", "_tcp",
                                           CONFIG_OCS_NETWORK_HTTP_SERVER_PORT);
        if (code == status::StatusCode::OK) {
            mdns_provider_->add_service_txt_records("_http", "_tcp",
                                                    net::MDNSProvider::TxtRecordList {
                                                        {
                                                            "telemetry",
                                                            "/telemetry",
                                                        },
                                                        {
                                                            "registration",
                                                            "/registration",
                                                        },
                                                        {
                                                            "command_reboot",
                                                            "/commands/reboot",
                                                        },
                                                        {
                                                            "command_reload",
                                                            "/commands/reload",
                                                        },
                                                    });
        }
    }

    if (code != status::StatusCode::OK) {
        ESP_LOGE(log_tag, "failed to start mDNS service: code=%s",
                 status::code_to_str(code));
    }
}

} // namespace app
} // namespace ocs
