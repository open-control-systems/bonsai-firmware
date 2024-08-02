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

#include "scs/adc_reader.h"
#include "scs/control_pipeline.h"
#include "scs/control_task.h"
#include "scs/yl69_moisture_reader.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "control-pipeline";

} // namespace

ControlPipeline::ControlPipeline(scheduler::AsyncTaskScheduler& task_scheduler,
                                 scheduler::TimerStore& timer_store,
                                 ITelemetryWriter& telemetry_writer) {
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

    adc_reader_.reset(new (std::nothrow) AdcReader(AdcReader::Params {
        .channel = static_cast<adc_channel_t>(CONFIG_SMC_SENSOR_ADC_CHANNEL),
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    }));
    configASSERT(adc_reader_);

    moisture_reader_.reset(
        new (std::nothrow) YL69MoistureReader(CONFIG_SMC_SENSOR_THRESHOLD, *adc_reader_));
    configASSERT(moisture_reader_);

    control_task_.reset(new (std::nothrow)
                            ControlTask(*moisture_reader_, telemetry_writer));
    configASSERT(control_task_);

    relay_task_.reset(new (std::nothrow) io::OneshotGpio(*control_task_, *delay_gpio_));
    configASSERT(relay_task_);

    relay_task_async_ = task_scheduler.add(*control_task_);
    configASSERT(relay_task_async_);

    task_ = relay_task_.get();
    async_task_ = relay_task_async_.get();

    async_task_timer_.reset(new (std::nothrow) scheduler::HighResolutionTimer(
        *async_task_, "control-task", core::Second * CONFIG_SMC_READ_INTERVAL));
    configASSERT(async_task_timer_);

    timer_store.add(*async_task_timer_);
}

status::StatusCode ControlPipeline::start() {
    const auto code = task_->run();
    if (code != status::StatusCode::OK) {
        ESP_LOGE(log_tag, "failed to run control task: code=%s",
                 status::code_to_str(code));
    }

    return status::StatusCode::OK;
}

scheduler::ITask& ControlPipeline::get_control_task() {
    return *async_task_;
}

} // namespace app
} // namespace ocs
