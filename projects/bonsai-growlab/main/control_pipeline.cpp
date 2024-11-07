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
#include "ocs_pipeline/jsonfmt/soil_analog_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE
#include "ocs_pipeline/jsonfmt/ldr_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE
#include "ocs_pipeline/jsonfmt/soil_analog_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
#include "ocs_pipeline/jsonfmt/sht41_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
#include "ocs_pipeline/jsonfmt/bme280_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE

#include "ocs_algo/bit_ops.h"
#include "ocs_io/adc/default_store.h"
#include "ocs_io/i2c/master_store.h"
#include "ocs_io/spi/master_store.h"
#include "ocs_io/spi/types.h"

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
    config.pin_bit_mask = algo::BitOps::mask(gpio);
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
    adc_store_.reset(new (std::nothrow)
                         io::adc::DefaultStore(io::adc::DefaultStore::Params {
                             .unit = ADC_UNIT_1,
                             .atten = ADC_ATTEN_DB_12,
                             .bitwidth = ADC_BITWIDTH_10,
                         }));
    configASSERT(adc_store_);

    i2c_master_store_.reset(new (
        std::nothrow) io::i2c::MasterStore(io::i2c::MasterStore::Params {
        .sda = static_cast<io::gpio::Gpio>(CONFIG_BONSAI_FIRMWARE_I2C_MASTER_SDA_GPIO),
        .scl = static_cast<io::gpio::Gpio>(CONFIG_BONSAI_FIRMWARE_I2C_MASTER_SCL_GPIO),
    }));
    configASSERT(i2c_master_store_);

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

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE
    yl69_sensor_pipeline_.reset(
        new (std::nothrow) sensor::soil::AnalogRelaySensorPipeline(
            clock, *adc_store_, storage_builder, reboot_handler, task_scheduler,
            "soil-yl69",
            sensor::soil::AnalogRelaySensorPipeline::Params {
                .sensor =
                    sensor::soil::AnalogSensor::Params {
                        .value_min = CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_VALUE_MIN,
                        .value_max = CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_VALUE_MAX,
                    },
                .adc_channel = static_cast<io::adc::Channel>(
                    CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ADC_CHANNEL),
                .fsm_block =
                    control::FsmBlockPipeline::Params {
                        .state_save_interval = core::Duration::hour * 2,
                        .state_interval_resolution = core::Duration::second,
                    },
                .read_interval = core::Duration::second
                    * CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_READ_INTERVAL,
                .relay_gpio = static_cast<io::gpio::Gpio>(
                    CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_RELAY_GPIO),
                .power_on_delay_interval =
                    (1000 * CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_POWER_ON_DELAY_INTERVAL)
                    / portTICK_PERIOD_MS,
            }));
    configASSERT(yl69_sensor_pipeline_);

    yl69_sensor_json_formatter_.reset(new (std::nothrow)
                                          pipeline::jsonfmt::SoilAnalogSensorFormatter(
                                              yl69_sensor_pipeline_->get_sensor()));
    configASSERT(yl69_sensor_json_formatter_);

    telemetry_formatter.add(*yl69_sensor_json_formatter_);

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
                },
            .adc_channel = static_cast<io::adc::Channel>(
                CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ADC_CHANNEL),
            .read_interval =
                core::Duration::second * CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_READ_INTERVAL,
        }));
    configASSERT(ldr_sensor_pipeline_);

    ldr_sensor_json_formatter_.reset(
        new (std::nothrow)
            pipeline::jsonfmt::LdrSensorFormatter(ldr_sensor_pipeline_->get_sensor()));
    configASSERT(ldr_sensor_json_formatter_);

    telemetry_formatter.add(*ldr_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE
    capacitive_sensor_sepipeline_.reset(
        new (std::nothrow) sensor::soil::AnalogSensorPipeline(
            clock, *adc_store_, storage_builder, reboot_handler, task_scheduler,
            "soil_capacitive",
            sensor::soil::AnalogSensorPipeline::Params {
                .sensor =
                    sensor::soil::AnalogSensor::Params {
                        .value_min =
                            CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_VALUE_MIN,
                        .value_max =
                            CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_VALUE_MAX,
                    },
                .adc_channel = static_cast<io::adc::Channel>(
                    CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ADC_CHANNEL),
                .fsm_block =
                    control::FsmBlockPipeline::Params {
                        .state_save_interval = core::Duration::hour * 2,
                        .state_interval_resolution = core::Duration::second,
                    },
                .read_interval = core::Duration::second
                    * CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_READ_INTERVAL,
            }));
    configASSERT(capacitive_sensor_sepipeline_);

    capacitive_sensor_json_formatter_.reset(
        new (std::nothrow) pipeline::jsonfmt::SoilAnalogSensorFormatter(
            capacitive_sensor_sepipeline_->get_sensor()));
    configASSERT(capacitive_sensor_json_formatter_);

    telemetry_formatter.add(*capacitive_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
    sht41_sensor_pipeline_.reset(new (std::nothrow) sensor::sht41::SensorPipeline(
        *i2c_master_store_, task_scheduler,
        sensor::sht41::SensorPipeline::Params {
            .read_interval = core::Duration::second
                * CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_READ_INTERVAL,
        }));
    configASSERT(sht41_sensor_pipeline_);

    sht41_sensor_json_formatter_.reset(new (std::nothrow)
                                           pipeline::jsonfmt::SHT41SensorFormatter(
                                               sht41_sensor_pipeline_->get_sensor()));
    configASSERT(sht41_sensor_json_formatter_);

    telemetry_formatter.add(*sht41_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_SPI_ENABLE
    bme280_spi_sensor_pipeline_.reset(
        new (std::nothrow) sensor::bme280::SpiSensorPipeline(
            task_scheduler, *spi_master_store_,
            sensor::bme280::SpiSensorPipeline::Params {
                .read_interval = CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_READ_INTERVAL
                    * core::Duration::second,
                .cs_gpio = static_cast<io::gpio::Gpio>(
                    CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_CS_GPIO),
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
        new (std::nothrow) pipeline::jsonfmt::BME280SensorFormatter(
            bme280_spi_sensor_pipeline_->get_sensor()));
    configASSERT(bme280_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_SPI_ENABLE

    telemetry_formatter.add(*bme280_sensor_json_formatter_);
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
}

} // namespace bonsai
} // namespace ocs