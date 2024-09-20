/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>

#include "sdkconfig.h"

#ifdef CONFIG_SCS_SENSOR_YL69_ENABLE
#include "ocs_pipeline/yl69/json_formatter.h"
#include "ocs_sensor/yl69/safe_sensor_task.h"
#endif // CONFIG_SCS_SENSOR_YL69_ENABLE

#ifdef CONFIG_SCS_SENSOR_LDR_ENABLE
#include "ocs_pipeline/ldr/json_formatter.h"
#include "ocs_sensor/ldr/sensor_task.h"
#endif // CONFIG_SCS_SENSOR_LDR_ENABLE

#ifdef CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE
#include "ocs_pipeline/ds18b20/json_formatter.h"
#include "ocs_sensor/ds18b20/sensor_task.h"
#include "ocs_system/delayer_configuration.h"
#endif // CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE

#ifdef CONFIG_SCS_SENSOR_CAPACITIVE_V1_2_ENABLE
#include "ocs_pipeline/yl69/json_formatter.h"
#include "ocs_sensor/yl69/default_sensor_task.h"
#endif // CONFIG_SCS_SENSOR_CAPACITIVE_V1_2_ENABLE

#include "ocs_core/bit_ops.h"
#include "ocs_scheduler/high_resolution_timer.h"

#include "scs/control_pipeline.h"

namespace ocs {
namespace scs {

namespace {

#if defined(CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE)                           \
    || defined(CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)
void configure_onewire_gpio(int gpio) {
    gpio_config_t config;
    memset(&config, 0, sizeof(config));

    // disable interrupt
    config.intr_type = GPIO_INTR_DISABLE;
    // output/input mode is controlled by the bus.
    config.mode = GPIO_MODE_DISABLE;
    // bit mask of the pins that you want to set,
    config.pin_bit_mask = core::BitOps::mask(gpio);
    // disable pull-down mode
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // enable pull-up mode
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    // configure GPIO with the given settings
    gpio_config(&config);
}
#endif // defined(CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE) ||
       // defined(CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)

#ifdef CONFIG_SCS_SENSOR_YL69_ENABLE
void configure_relay_gpio(int gpio) {
    gpio_config_t config;
    memset(&config, 0, sizeof(config));

    // disable interrupt
    config.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    config.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,
    config.pin_bit_mask = core::BitOps::mask(gpio);
    // enable pull-down mode
    config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    // disable pull-up mode
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    // configure GPIO with the given settings
    gpio_config(&config);
}
#endif // CONFIG_SCS_SENSOR_YL69_ENABLE

} // namespace

ControlPipeline::ControlPipeline(core::IClock& clock,
                                 storage::StorageBuilder& storage_builder,
                                 system::FanoutRebootHandler& reboot_handler,
                                 scheduler::AsyncTaskScheduler& task_scheduler,
                                 scheduler::TimerStore& timer_store,
                                 diagnostic::BasicCounterHolder& counter_holder,
                                 fmt::json::FanoutFormatter& telemetry_formatter) {
    adc_store_.reset(new (std::nothrow) io::AdcStore(io::AdcStore::Params {
        .unit = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    }));
    configASSERT(adc_store_);

    counter_storage_ = storage_builder.make("soil_counter");
    configASSERT(counter_storage_);

    fanout_task_.reset(new (std::nothrow) scheduler::FanoutTask());
    configASSERT(fanout_task_);

#ifdef CONFIG_SCS_SENSOR_YL69_ENABLE
    yl69_sensor_task_.reset(new (std::nothrow) sensor::yl69::SafeSensorTask(
        clock, *adc_store_, *counter_storage_, reboot_handler, task_scheduler,
        timer_store, counter_holder, "soil-YL69", "soil-YL69-sensor-task",
        "soil-YL69-task",
        sensor::yl69::SafeSensorTask::Params {
            .sensor =
                sensor::yl69::Sensor::Params {
                    .value_min = CONFIG_SCS_SENSOR_YL69_VALUE_MIN,
                    .value_max = CONFIG_SCS_SENSOR_YL69_VALUE_MAX,
                    .adc_channel =
                        static_cast<adc_channel_t>(CONFIG_SCS_SENSOR_YL69_ADC_CHANNEL),
                },
            .read_interval = core::Second * CONFIG_SCS_SENSOR_YL69_READ_INTERVAL,
            .relay_gpio = static_cast<gpio_num_t>(CONFIG_SCS_SENSOR_YL69_RELAY_GPIO),
            .power_on_delay_interval =
                (1000 * CONFIG_SCS_SENSOR_YL69_POWER_ON_DELAY_INTERVAL)
                / portTICK_PERIOD_MS,
        }));
    configASSERT(yl69_sensor_task_);

    fanout_task_->add(*yl69_sensor_task_);

    yl69_sensor_json_formatter_.reset(new (std::nothrow) pipeline::yl69::JsonFormatter(
        yl69_sensor_task_->get_sensor(), true));
    configASSERT(yl69_sensor_json_formatter_);

    telemetry_formatter.add(*yl69_sensor_json_formatter_);

    configure_relay_gpio(CONFIG_SCS_SENSOR_YL69_RELAY_GPIO);
#endif // CONFIG_SCS_SENSOR_YL69_ENABLE

#ifdef CONFIG_SCS_SENSOR_LDR_ENABLE
    ldr_sensor_task_.reset(new (std::nothrow) sensor::ldr::SensorTask(
        *adc_store_, task_scheduler, timer_store, "soil-LDR", "soil-LDR-task",
        sensor::ldr::SensorTask::Params {
            .sensor =
                sensor::ldr::Sensor::Params {
                    .value_min = CONFIG_SCS_SENSOR_LDR_VALUE_MIN,
                    .value_max = CONFIG_SCS_SENSOR_LDR_VALUE_MAX,
                    .adc_channel =
                        static_cast<adc_channel_t>(CONFIG_SCS_SENSOR_LDR_ADC_CHANNEL),
                },
            .read_interval = core::Second * CONFIG_SCS_SENSOR_LDR_READ_INTERVAL,
        }));
    configASSERT(ldr_sensor_task_);

    fanout_task_->add(*ldr_sensor_task_);

    ldr_sensor_json_formatter_.reset(new (std::nothrow) pipeline::ldr::JsonFormatter(
        ldr_sensor_task_->get_sensor(), true));
    configASSERT(ldr_sensor_json_formatter_);

    telemetry_formatter.add(*ldr_sensor_json_formatter_);
#endif // CONFIG_SCS_SENSOR_LDR_ENABLE

    ds18b20_sensor_storage_ = storage_builder.make("ds18b20_sensors");
    configASSERT(ds18b20_sensor_storage_);

    ds18b20_sensor_store_.reset(new (std::nothrow) sensor::ds18b20::Store(16));
    configASSERT(ds18b20_sensor_store_);

#ifdef CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE
    soil_temperature_sensor_task_.reset(new (std::nothrow) sensor::ds18b20::SensorTask(
        timer_store, task_scheduler, *ds18b20_sensor_storage_, *ds18b20_sensor_store_,
        "soil_temp", "soil-temperature-task",
        sensor::ds18b20::SensorTask::Params {
            .read_interval =
                core::Second * CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_READ_INTERVAL,
            .data_pin = static_cast<gpio_num_t>(
                CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_DATA_GPIO),
        }));
    configASSERT(soil_temperature_sensor_task_);

    fanout_task_->add(*soil_temperature_sensor_task_);

    soil_temperature_sensor_json_formatter_.reset(
        new (std::nothrow) pipeline::ds18b20::JsonFormatter(
            soil_temperature_sensor_task_->get_sensor()));
    configASSERT(soil_temperature_sensor_json_formatter_);

    telemetry_formatter.add(*soil_temperature_sensor_json_formatter_);

    configure_onewire_gpio(CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_DATA_GPIO);
#endif // CONFIG_SCS_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE

#ifdef CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE
    outside_temperature_sensor_task_.reset(new (std::nothrow) sensor::ds18b20::SensorTask(
        timer_store, task_scheduler, *ds18b20_sensor_storage_, *ds18b20_sensor_store_,
        "outside_temp", "outside-temperature-task",
        sensor::ds18b20::SensorTask::Params {
            .read_interval = core::Second
                * CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_READ_INTERVAL,
            .data_pin = static_cast<gpio_num_t>(
                CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_DATA_GPIO),
        }));
    configASSERT(outside_temperature_sensor_task_);

    fanout_task_->add(*outside_temperature_sensor_task_);

    outside_temperature_sensor_json_formatter_.reset(
        new (std::nothrow) pipeline::ds18b20::JsonFormatter(
            outside_temperature_sensor_task_->get_sensor()));
    configASSERT(outside_temperature_sensor_json_formatter_);

    telemetry_formatter.add(*outside_temperature_sensor_json_formatter_);

    configure_onewire_gpio(CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_DATA_GPIO);
#endif // CONFIG_SCS_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE

    if (soil_temperature_sensor_task_ || outside_temperature_sensor_task_) {
        ds18b20_sensor_store_task_ = task_scheduler.add(*ds18b20_sensor_store_);
        configASSERT(ds18b20_sensor_store_task_);

        ds18b20_sensor_store_timer_.reset(
            new (std::nothrow) scheduler::HighResolutionTimer(*ds18b20_sensor_store_task_,
                                                              "DS18B20-store-timer",
                                                              core::Millisecond * 500));
        configASSERT(ds18b20_sensor_store_timer_);

        timer_store.add(*ds18b20_sensor_store_timer_);
    }

#ifdef CONFIG_SCS_SENSOR_CAPACITIVE_V1_2_ENABLE
    capacitive_sensor_task_.reset(new (std::nothrow) sensor::yl69::DefaultSensorTask(
        clock, *adc_store_, *counter_storage_, reboot_handler, task_scheduler,
        timer_store, counter_holder, "soil-capacitive", "soil-capacitive-sensor-task",
        "soil-capacitive-task",
        sensor::yl69::DefaultSensorTask::Params {
            .sensor =
                sensor::yl69::Sensor::Params {
                    .value_min = CONFIG_SCS_SENSOR_CAPACITIVE_V1_2_VALUE_MIN,
                    .value_max = CONFIG_SCS_SENSOR_CAPACITIVE_V1_2_VALUE_MAX,
                    .adc_channel = static_cast<adc_channel_t>(
                        CONFIG_SCS_SENSOR_CAPACITIVE_V1_2_ADC_CHANNEL),
                },
            .read_interval =
                core::Second * CONFIG_SCS_SENSOR_CAPACITIVE_V1_2_READ_INTERVAL,
        }));
    configASSERT(capacitive_sensor_task_);

    fanout_task_->add(*capacitive_sensor_task_);

    capacitive_sensor_json_formatter_.reset(
        new (std::nothrow)
            pipeline::yl69::JsonFormatter(capacitive_sensor_task_->get_sensor(), true));
    configASSERT(capacitive_sensor_json_formatter_);

    telemetry_formatter.add(*capacitive_sensor_json_formatter_);
#endif // CONFIG_SCS_SENSOR_CAPACITIVE_V1_2_ENABLE
}

status::StatusCode ControlPipeline::start() {
    return fanout_task_->run();
}

sensor::ds18b20::Store& ControlPipeline::get_ds18b20_store() {
    return *ds18b20_sensor_store_;
}

} // namespace scs
} // namespace ocs
