/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>

#include "sdkconfig.h"

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE
#include "ocs_pipeline/yl69/json_formatter.h"
#include "ocs_sensor/yl69/relay_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE
#include "ocs_pipeline/ldr/json_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE
#include "ocs_pipeline/yl69/json_formatter.h"
#include "ocs_sensor/yl69/default_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
#include "ocs_pipeline/sht41/json_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
#include "ocs_pipeline/bme280/json_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE

#include "ocs_core/bit_ops.h"
#include "ocs_i2c/master_store.h"
#include "ocs_spi/master_store.h"
#include "ocs_spi/types.h"

#include "control_pipeline.h"

namespace ocs {
namespace bonsai {

namespace {

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE
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
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE

} // namespace

ControlPipeline::ControlPipeline(core::IClock& clock,
                                 storage::StorageBuilder& storage_builder,
                                 system::FanoutRebootHandler& reboot_handler,
                                 scheduler::ITaskScheduler& task_scheduler,
                                 fmt::json::FanoutFormatter& telemetry_formatter) {
    adc_store_.reset(new (std::nothrow) io::AdcStore(io::AdcStore::Params {
        .unit = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    }));
    configASSERT(adc_store_);

    i2c_master_store_.reset(new (std::nothrow) i2c::MasterStore(i2c::MasterStore::Params {
        .sda = static_cast<gpio_num_t>(CONFIG_BONSAI_FIRMWARE_I2C_MASTER_SDA_GPIO),
        .scl = static_cast<gpio_num_t>(CONFIG_BONSAI_FIRMWARE_I2C_MASTER_SCL_GPIO),
    }));
    configASSERT(i2c_master_store_);

    spi_master_store_.reset(new (std::nothrow) spi::MasterStore(spi::MasterStore::Params {
        .mosi = static_cast<gpio_num_t>(CONFIG_BONSAI_FIRMWARE_SPI_MASTER_MOSI_GPIO),
        .miso = static_cast<gpio_num_t>(CONFIG_BONSAI_FIRMWARE_SPI_MASTER_MISO_GPIO),
        .sclk = static_cast<gpio_num_t>(CONFIG_BONSAI_FIRMWARE_SPI_MASTER_SCLK_GPIO),
        .max_transfer_size =
            static_cast<gpio_num_t>(CONFIG_BONSAI_FIRMWARE_SPI_MASTER_MAX_TRANSFER_SIZE),
        .host_id = spi::VSPI,
    }));
    configASSERT(spi_master_store_);

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE
    yl69_sensor_pipeline_.reset(new (std::nothrow) sensor::yl69::RelayPipeline(
        clock, *adc_store_, storage_builder, reboot_handler, task_scheduler, "soil-yl69",
        sensor::yl69::RelayPipeline::Params {
            .sensor =
                sensor::yl69::Sensor::Params {
                    .value_min = CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_VALUE_MIN,
                    .value_max = CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_VALUE_MAX,
                    .adc_channel = static_cast<adc_channel_t>(
                        CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ADC_CHANNEL),
                },
            .fsm_block =
                control::FsmBlockPipeline::Params {
                    .state_save_interval = core::Duration::hour * 2,
                    .state_interval_resolution = core::Duration::second,
                },
            .read_interval =
                core::Duration::second * CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_READ_INTERVAL,
            .relay_gpio =
                static_cast<gpio_num_t>(CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_RELAY_GPIO),
            .power_on_delay_interval =
                (1000 * CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_POWER_ON_DELAY_INTERVAL)
                / portTICK_PERIOD_MS,
        }));
    configASSERT(yl69_sensor_pipeline_);

    yl69_sensor_json_formatter_.reset(new (std::nothrow) pipeline::yl69::JsonFormatter(
        yl69_sensor_pipeline_->get_sensor(), false));
    configASSERT(yl69_sensor_json_formatter_);

    yl69_sensor_field_formatter_.reset(new (std::nothrow) fmt::json::FieldFormatter(
        "soil_yl69", fmt::json::FieldFormatter::Type::Object));
    configASSERT(yl69_sensor_field_formatter_);

    yl69_sensor_field_formatter_->add(*yl69_sensor_json_formatter_);

    telemetry_formatter.add(*yl69_sensor_field_formatter_);

    configure_relay_gpio(CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_RELAY_GPIO);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE
    ldr_sensor_pipeline_.reset(new (std::nothrow) sensor::ldr::SensorPipeline(
        *adc_store_, task_scheduler, "soil_ldr",
        sensor::ldr::SensorPipeline::Params {
            .sensor =
                sensor::ldr::Sensor::Params {
                    .value_min = CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_VALUE_MIN,
                    .value_max = CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_VALUE_MAX,
                    .adc_channel = static_cast<adc_channel_t>(
                        CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ADC_CHANNEL),
                },
            .read_interval =
                core::Duration::second * CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_READ_INTERVAL,
        }));
    configASSERT(ldr_sensor_pipeline_);

    ldr_sensor_json_formatter_.reset(new (std::nothrow) pipeline::ldr::JsonFormatter(
        ldr_sensor_pipeline_->get_sensor(), false));
    configASSERT(ldr_sensor_json_formatter_);

    ldr_sensor_field_formatter_.reset(new (std::nothrow) fmt::json::FieldFormatter(
        "soil_ldr", fmt::json::FieldFormatter::Type::Object));
    configASSERT(ldr_sensor_field_formatter_);

    ldr_sensor_field_formatter_->add(*ldr_sensor_json_formatter_);

    telemetry_formatter.add(*ldr_sensor_field_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE
    capacitive_sensor_sepipeline_.reset(new (std::nothrow) sensor::yl69::DefaultPipeline(
        clock, *adc_store_, storage_builder, reboot_handler, task_scheduler,
        "soil_capacitive",
        sensor::yl69::DefaultPipeline::Params {
            .sensor =
                sensor::yl69::Sensor::Params {
                    .value_min = CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_VALUE_MIN,
                    .value_max = CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_VALUE_MAX,
                    .adc_channel = static_cast<adc_channel_t>(
                        CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ADC_CHANNEL),
                },
            .fsm_block =
                control::FsmBlockPipeline::Params {
                    .state_save_interval = core::Duration::hour * 2,
                    .state_interval_resolution = core::Duration::second,
                },
            .read_interval = core::Duration::second
                * CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_READ_INTERVAL,
        }));
    configASSERT(capacitive_sensor_sepipeline_);

    capacitive_sensor_field_formatter_.reset(new (std::nothrow) fmt::json::FieldFormatter(
        "soil_capacitive", fmt::json::FieldFormatter::Type::Object));
    configASSERT(capacitive_sensor_field_formatter_);

    capacitive_sensor_json_formatter_.reset(
        new (std::nothrow) pipeline::yl69::JsonFormatter(
            capacitive_sensor_sepipeline_->get_sensor(), false));
    configASSERT(capacitive_sensor_json_formatter_);

    capacitive_sensor_field_formatter_->add(*capacitive_sensor_json_formatter_);

    telemetry_formatter.add(*capacitive_sensor_field_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
    sht41_sensor_pipeline_.reset(new (std::nothrow) sensor::sht41::SensorPipeline(
        *i2c_master_store_, task_scheduler,
        sensor::sht41::SensorPipeline::Params {
            .read_interval = core::Duration::second
                * CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_READ_INTERVAL,
        }));
    configASSERT(sht41_sensor_pipeline_);

    sht41_sensor_field_formatter_.reset(new (std::nothrow) fmt::json::FieldFormatter(
        "SHT41", fmt::json::FieldFormatter::Type::Object));
    configASSERT(sht41_sensor_field_formatter_);

    sht41_sensor_json_formatter_.reset(new (std::nothrow) pipeline::sht41::JsonFormatter(
        sht41_sensor_pipeline_->get_sensor(), false));
    configASSERT(sht41_sensor_json_formatter_);

    sht41_sensor_field_formatter_->add(*sht41_sensor_json_formatter_);

    telemetry_formatter.add(*sht41_sensor_field_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_SPI_ENABLE
    bme280_spi_sensor_pipeline_.reset(
        new (std::nothrow) sensor::bme280::SpiSensorPipeline(
            task_scheduler, *spi_master_store_,
            sensor::bme280::SpiSensorPipeline::Params {
                .read_interval = CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_READ_INTERVAL
                    * core::Duration::second,
                .cs_gpio =
                    static_cast<gpio_num_t>(CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_CS_GPIO),
                .sensor =
                    sensor::bme280::Sensor::Params {
                        .operation_mode = sensor::bme280::Sensor::OperationMode::Forced,
                        .pressure_oversampling =
                            sensor::bme280::Sensor::OversamplingMode::One,
                        .temperature_oversampling =
                            sensor::bme280::Sensor::OversamplingMode::One,
                        .humidity_oversampling =
                            sensor::bme280::Sensor::OversamplingMode::One,
                        .pressure_resolution = 2,
                        .pressure_decimal_places = 2,
                        .humidity_decimal_places = 2,
                    },
            }));
    configASSERT(bme280_spi_sensor_pipeline_);

    bme280_sensor_json_formatter_.reset(
        new (std::nothrow) pipeline::bme280::JsonFormatter(
            bme280_spi_sensor_pipeline_->get_sensor(), false));
    configASSERT(bme280_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_SPI_ENABLE

    bme280_sensor_field_formatter_.reset(new (std::nothrow) fmt::json::FieldFormatter(
        "BME280", fmt::json::FieldFormatter::Type::Object));
    configASSERT(bme280_sensor_field_formatter_);

    bme280_sensor_field_formatter_->add(*bme280_sensor_json_formatter_);

    telemetry_formatter.add(*bme280_sensor_field_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
}

} // namespace bonsai
} // namespace ocs
