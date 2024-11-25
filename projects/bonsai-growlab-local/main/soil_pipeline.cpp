/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>

#include "freertos/FreeRTOSConfig.h"

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE
#include "ocs_pipeline/jsonfmt/soil_analog_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE
#include "ocs_pipeline/jsonfmt/soil_analog_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE

#include "ocs_algo/bit_ops.h"

#include "soil_pipeline.h"

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

} // namespace

SoilPipeline::SoilPipeline(io::adc::IStore& adc_store,
                           core::IClock& clock,
                           storage::StorageBuilder& storage_builder,
                           system::FanoutRebootHandler& reboot_handler,
                           scheduler::ITaskScheduler& task_scheduler,
                           fmt::json::FanoutFormatter& telemetry_formatter) {
#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE
    analog_relay_sensor_pipeline_.reset(
        new (std::nothrow) sensor::soil::AnalogRelaySensorPipeline(
            clock, adc_store, storage_builder, reboot_handler, task_scheduler,
            "soil_analog_relay",
            sensor::soil::AnalogRelaySensorPipeline::Params {
                .sensor =
                    sensor::soil::AnalogSensor::Params {
                        .value_min =
                            CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_VALUE_MIN,
                        .value_max =
                            CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_VALUE_MAX,
                    },
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
    configASSERT(analog_relay_sensor_pipeline_);

    analog_relay_sensor_json_formatter_.reset(
        new (std::nothrow) pipeline::jsonfmt::SoilAnalogSensorFormatter(
            analog_relay_sensor_pipeline_->get_sensor()));
    configASSERT(analog_relay_sensor_json_formatter_);

    telemetry_formatter.add(*analog_relay_sensor_json_formatter_);

    configure_relay_gpio(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_GPIO);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE
    analog_sensor_pipeline_.reset(new (std::nothrow) sensor::soil::AnalogSensorPipeline(
        clock, adc_store, storage_builder, reboot_handler, task_scheduler, "soil_analog",
        sensor::soil::AnalogSensorPipeline::Params {
            .sensor =
                sensor::soil::AnalogSensor::Params {
                    .value_min = CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_VALUE_MIN,
                    .value_max = CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_VALUE_MAX,
                },
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
    configASSERT(analog_sensor_pipeline_);

    analog_sensor_json_formatter_.reset(new (std::nothrow)
                                            pipeline::jsonfmt::SoilAnalogSensorFormatter(
                                                analog_sensor_pipeline_->get_sensor()));
    configASSERT(analog_sensor_json_formatter_);

    telemetry_formatter.add(*analog_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE
}

} // namespace bonsai
} // namespace ocs
