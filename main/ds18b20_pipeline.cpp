/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>

#include "freertos/FreeRTOSConfig.h"

#include "ocs_algo/bit_ops.h"
#include "ocs_pipeline/jsonfmt/ds18b20_sensor_formatter.h"

#include "ds18b20_pipeline.h"

namespace ocs {
namespace bonsai {

namespace {

#if defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE)               \
    || defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)
void configure_onewire_gpio(int gpio) {
    gpio_config_t config;
    memset(&config, 0, sizeof(config));

    // disable interrupt
    config.intr_type = GPIO_INTR_DISABLE;
    // output/input mode is controlled by the bus.
    config.mode = GPIO_MODE_DISABLE;
    // bit mask of the pins that you want to set,
    config.pin_bit_mask = algo::BitOps::mask(gpio);
    // disable pull-down mode
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // enable pull-up mode
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    // configure GPIO with the given settings
    gpio_config(&config);
}
#endif // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE) ||
       // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)

} // namespace

DS18B20Pipeline::DS18B20Pipeline(core::IClock& clock,
                                 storage::StorageBuilder& storage_builder,
                                 scheduler::ITaskScheduler& task_scheduler,
                                 fmt::json::FanoutFormatter& telemetry_formatter) {
    storage_ = storage_builder.make("ds18b20_sensors");
    configASSERT(storage_);

    store_.reset(new (std::nothrow) sensor::ds18b20::Store(8));
    configASSERT(store_);

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE
    soil_temperature_pipeline_.reset(new (std::nothrow) sensor::ds18b20::SensorPipeline(
        task_scheduler, *storage_, *store_, "soil_temp",
        sensor::ds18b20::SensorPipeline::Params {
            .read_interval = core::Duration::second
                * CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_READ_INTERVAL,
            .data_pin = static_cast<io::gpio::Gpio>(
                CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_DATA_GPIO),
        }));
    configASSERT(soil_temperature_pipeline_);

    soil_temperature_json_formatter_.reset(
        new (std::nothrow) pipeline::jsonfmt::DS18B20SensorFormatter(
            soil_temperature_pipeline_->get_sensor()));
    configASSERT(soil_temperature_json_formatter_);

    telemetry_formatter.add(*soil_temperature_json_formatter_);

    configure_onewire_gpio(
        CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_DATA_GPIO);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE
    outside_temperature_pipeline_.reset(new (std::nothrow) sensor::ds18b20::SensorPipeline(
        task_scheduler, *storage_, *store_, "outside_temp",
        sensor::ds18b20::SensorPipeline::Params {
            .read_interval = core::Duration::second
                * CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_READ_INTERVAL,
            .data_pin = static_cast<io::gpio::Gpio>(
                CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_DATA_GPIO),
        }));
    configASSERT(outside_temperature_pipeline_);

    outside_temperature_json_formatter_.reset(
        new (std::nothrow) pipeline::jsonfmt::DS18B20SensorFormatter(
            outside_temperature_pipeline_->get_sensor()));
    configASSERT(outside_temperature_json_formatter_);

    telemetry_formatter.add(*outside_temperature_json_formatter_);

    configure_onewire_gpio(
        CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_DATA_GPIO);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE

    configASSERT(task_scheduler.add(*store_, "ds18b20_store_task", core::Duration::second)
                 == status::StatusCode::OK);
}

sensor::ds18b20::Store& DS18B20Pipeline::get_store() {
    return *store_;
}

} // namespace bonsai
} // namespace ocs
