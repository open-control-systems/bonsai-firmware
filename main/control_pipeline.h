/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE
#include "ocs_sensor/ldr/sensor_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE
#include "ocs_sensor/yl69/relay_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE
#include "ocs_sensor/yl69/default_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
#include "ocs_sensor/sht41/sensor_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_SPI_ENABLE
#include "ocs_sensor/bme280/spi_sensor_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_SPI_ENABLE

#include "ocs_core/iclock.h"
#include "ocs_core/noncopyable.h"
#include "ocs_fmt/json/fanout_formatter.h"
#include "ocs_fmt/json/field_formatter.h"
#include "ocs_i2c/istore.h"
#include "ocs_io/adc_store.h"
#include "ocs_scheduler/itask_scheduler.h"
#include "ocs_spi/istore.h"
#include "ocs_storage/istorage.h"
#include "ocs_storage/storage_builder.h"
#include "ocs_system/fanout_reboot_handler.h"

namespace ocs {

namespace bonsai {

class ControlPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    ControlPipeline(core::IClock& clock,
                    storage::StorageBuilder& storage_builder,
                    system::FanoutRebootHandler& reboot_handler,
                    scheduler::ITaskScheduler& task_scheduler,
                    fmt::json::FanoutFormatter& telemetry_formatter);

private:
    std::unique_ptr<io::AdcStore> adc_store_;
    std::unique_ptr<i2c::IStore> i2c_master_store_;
    std::unique_ptr<spi::IStore> spi_master_store_;

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE
    std::unique_ptr<sensor::yl69::RelayPipeline> yl69_sensor_pipeline_;
    std::unique_ptr<fmt::json::FieldFormatter> yl69_sensor_field_formatter_;
    std::unique_ptr<fmt::json::IFormatter> yl69_sensor_json_formatter_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_YL69_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE
    std::unique_ptr<sensor::ldr::SensorPipeline> ldr_sensor_pipeline_;
    std::unique_ptr<fmt::json::FieldFormatter> ldr_sensor_field_formatter_;
    std::unique_ptr<fmt::json::IFormatter> ldr_sensor_json_formatter_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_LDR_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE
    std::unique_ptr<sensor::yl69::DefaultPipeline> capacitive_sensor_sepipeline_;
    std::unique_ptr<fmt::json::FieldFormatter> capacitive_sensor_field_formatter_;
    std::unique_ptr<fmt::json::IFormatter> capacitive_sensor_json_formatter_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_CAPACITIVE_V1_2_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
    std::unique_ptr<sensor::sht41::SensorPipeline> sht41_sensor_pipeline_;
    std::unique_ptr<fmt::json::FieldFormatter> sht41_sensor_field_formatter_;
    std::unique_ptr<fmt::json::IFormatter> sht41_sensor_json_formatter_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_SPI_ENABLE
    std::unique_ptr<sensor::bme280::SpiSensorPipeline> bme280_spi_sensor_pipeline_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_SPI_ENABLE
    std::unique_ptr<fmt::json::IFormatter> bme280_sensor_json_formatter_;
    std::unique_ptr<fmt::json::FieldFormatter> bme280_sensor_field_formatter_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_BME280_ENABLE
};

} // namespace bonsai
} // namespace ocs
