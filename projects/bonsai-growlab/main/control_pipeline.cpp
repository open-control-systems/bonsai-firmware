/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>

#include "sdkconfig.h"

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE
#include "ocs_pipeline/jsonfmt/ldr_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
#include "ocs_pipeline/jsonfmt/sht41_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
#include "ocs_pipeline/jsonfmt/bme280_sensor_formatter.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE

#include "ocs_io/i2c/master_store.h"
#include "ocs_io/spi/master_store.h"
#include "ocs_io/spi/types.h"

#include "control_pipeline.h"

namespace ocs {
namespace bonsai {

ControlPipeline::ControlPipeline(io::adc::IStore& adc_store,
                                 core::IClock& clock,
                                 storage::StorageBuilder& storage_builder,
                                 system::FanoutRebootHandler& reboot_handler,
                                 scheduler::ITaskScheduler& task_scheduler,
                                 fmt::json::FanoutFormatter& telemetry_formatter) {
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

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE
    ldr_sensor_pipeline_.reset(new (std::nothrow) sensor::ldr::SensorPipeline(
        adc_store, task_scheduler, "soil_ldr",
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
