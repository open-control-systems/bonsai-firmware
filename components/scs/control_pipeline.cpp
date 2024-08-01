/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_log.h"

#include "ocs_io/default_gpio.h"
#include "ocs_io/delay_gpio.h"
#include "ocs_io/oneshot_gpio.h"
#include "ocs_scheduler/high_resolution_timer.h"
#include "ocs_status/code_to_str.h"
#include "ocs_system/default_clock.h"
#include "ocs_system/default_rebooter.h"
#include "ocs_system/delay_rebooter.h"
#include "ocs_system/reboot_task.h"
#include "scs/adc_reader.h"
#include "scs/console_telemetry_writer.h"
#include "scs/control_pipeline.h"
#include "scs/yl69_moisture_reader.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "control-pipeline";

} // namespace

ControlPipeline::ControlPipeline() {
    adc_reader_.reset(new (std::nothrow) AdcReader(AdcReader::Params {
        .channel = static_cast<adc_channel_t>(CONFIG_SMC_SENSOR_ADC_CHANNEL),
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    }));
    configASSERT(adc_reader_);

    moisture_reader_.reset(
        new (std::nothrow) YL69MoistureReader(CONFIG_SMC_SENSOR_THRESHOLD, *adc_reader_));
    configASSERT(moisture_reader_);

    flash_initializer_.reset(new (std::nothrow) storage::FlashInitializer());
    configASSERT(flash_initializer_);

    http_server_pipeline_.reset(new (std::nothrow) iot::HttpServerPipeline());
    configASSERT(http_server_pipeline_);

    default_clock_.reset(new (std::nothrow) system::DefaultClock());
    configASSERT(default_clock_);

    task_scheduler_.reset(new (std::nothrow) scheduler::AsyncTaskScheduler());
    configASSERT(task_scheduler_);

    timer_store_.reset(new (std::nothrow) scheduler::TimerStore());
    configASSERT(timer_store_);

    fanout_reboot_handler_.reset(new (std::nothrow) system::FanoutRebootHandler());
    configASSERT(fanout_reboot_handler_);

    default_rebooter_.reset(new (std::nothrow)
                                system::DefaultRebooter(*fanout_reboot_handler_));
    configASSERT(default_rebooter_);

    delay_rebooter_.reset(
        new (std::nothrow) system::DelayRebooter(pdMS_TO_TICKS(500), *default_rebooter_));
    configASSERT(delay_rebooter_);

    reboot_task_.reset(new (std::nothrow) system::RebootTask(*delay_rebooter_));
    configASSERT(reboot_task_);

    reboot_task_async_ = task_scheduler_->add(*reboot_task_);
    configASSERT(reboot_task_async_);

    fanout_telemetry_writer_.reset(new (std::nothrow) FanoutTelemetryWriter());
    configASSERT(fanout_telemetry_writer_);

    telemetry_holder_.reset(new (std::nothrow) TelemetryHolder());
    configASSERT(telemetry_holder_);

    fanout_telemetry_writer_->add(*telemetry_holder_);

    telemetry_formatter_.reset(new (std::nothrow) TelemetryFormatter());
    configASSERT(telemetry_formatter_);

    telemetry_formatter_->fanout().add(*telemetry_holder_);

    http_telemetry_handler_.reset(new (std::nothrow) HttpTelemetryHandler(
        http_server_pipeline_->server(), *telemetry_formatter_, "/telemetry",
        "http-telemetry-handler"));
    configASSERT(http_telemetry_handler_);

    console_telemetry_writer_.reset(new (std::nothrow)
                                        ConsoleTelemetryWriter(*telemetry_formatter_));
    configASSERT(console_telemetry_writer_);

    fanout_telemetry_writer_->add(*console_telemetry_writer_);

    gpio_config_.reset(new (std::nothrow) GpioConfig());
    configASSERT(gpio_config_);

    default_gpio_.reset(new (std::nothrow) io::DefaultGpio(
        "relay-control", static_cast<gpio_num_t>(CONFIG_SMC_RELAY_GPIO)));
    configASSERT(default_gpio_);

    delay_gpio_.reset(new (std::nothrow) io::DelayGpio(
        *default_gpio_,
        io::DelayGpio::Params {
            .flip_delay_interval = pdMS_TO_TICKS(0),
            .turn_on_delay_interval =
                (1000 * CONFIG_SMC_POWER_ON_DELAY_INTERVAL) / portTICK_PERIOD_MS,
            .turn_off_delay_interval = pdMS_TO_TICKS(0),
        }));
    configASSERT(delay_gpio_);

    soil_moisture_task_.reset(new (std::nothrow) SoilMoistureMonitor(
        *moisture_reader_, *fanout_telemetry_writer_));
    configASSERT(soil_moisture_task_);

    relay_task_.reset(new (std::nothrow)
                          io::OneshotGpio(*soil_moisture_task_, *delay_gpio_));
    configASSERT(relay_task_);

    relay_task_async_ = task_scheduler_->add(*soil_moisture_task_);
    configASSERT(relay_task_async_);

    soil_moisture_timer_.reset(new (std::nothrow) scheduler::HighResolutionTimer(
        *relay_task_async_, "soil-moisture-timer",
        core::Second * CONFIG_SMC_READ_INTERVAL));
    configASSERT(soil_moisture_timer_);

    timer_store_->add(*soil_moisture_timer_);

    http_command_handler_.reset(new (std::nothrow) HttpCommandHandler(
        http_server_pipeline_->server(), *reboot_task_async_, *relay_task_async_));
    configASSERT(http_command_handler_);

    registration_formatter_.reset(
        new (std::nothrow) RegistrationFormatter(http_server_pipeline_->network()));
    configASSERT(registration_formatter_);

    http_registration_handler_.reset(new (std::nothrow) HttpRegistrationHandler(
        http_server_pipeline_->server(), *registration_formatter_, "/registration",
        "http-registration-handler"));
    configASSERT(http_registration_handler_);

    storage_builder_.reset(new (std::nothrow) storage::StorageBuilder());
    configASSERT(storage_builder_);

    system_counter_storage_ = storage_builder_->make("system_counter");
    configASSERT(system_counter_storage_);

    counter_json_formatter_.reset(new (std::nothrow) iot::CounterJsonFormatter());
    configASSERT(counter_json_formatter_);

    system_counter_pipeline_.reset(new (std::nothrow) iot::SystemCounterPipeline(
        *default_clock_, *system_counter_storage_, *fanout_reboot_handler_,
        *task_scheduler_, *timer_store_, *counter_json_formatter_));
    configASSERT(system_counter_pipeline_);

    telemetry_formatter_->fanout().add(*counter_json_formatter_);

    soil_counter_storage_ = storage_builder_->make("soil_counter");
    configASSERT(soil_counter_storage_);

    soil_status_monitor_.reset(new (std::nothrow) SoilStatusMonitor(
        *default_clock_, *soil_counter_storage_, *fanout_reboot_handler_,
        *task_scheduler_, *timer_store_, *counter_json_formatter_));
    configASSERT(soil_status_monitor_);

    fanout_telemetry_writer_->add(*soil_status_monitor_);
}

void ControlPipeline::start() {
    // Read the soil moisture data at startup.
    auto code = relay_task_->run();
    if (code != status::StatusCode::OK) {
        ESP_LOGW(log_tag, "failed to run soil moisture task: code=%s",
                 status::code_to_str(code));
    }

    code = http_server_pipeline_->start();
    if (code != status::StatusCode::OK) {
        ESP_LOGE(log_tag, "failed to start HTTP server pipeline: code=%s",
                 status::code_to_str(code));
    } else {
        register_mdns_endpoints_();
    }

    configASSERT(timer_store_->start() == status::StatusCode::OK);
    task_scheduler_->run();
}

void ControlPipeline::register_mdns_endpoints_() {
    http_server_pipeline_->mdns().add_service_txt_records(
        "_http", "_tcp",
        net::MdnsProvider::TxtRecordList {
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
            {
                "commands",
                "/commands",
            },
        });
}

} // namespace app
} // namespace ocs
