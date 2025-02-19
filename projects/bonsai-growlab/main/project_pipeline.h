/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include "ocs_core/noncopyable.h"
#include "ocs_io/adc/istore.h"
#include "ocs_io/i2c/master_store_pipeline.h"
#include "ocs_net/basic_mdns_server.h"
#include "ocs_net/fanout_network_handler.h"
#include "ocs_net/mdns_config.h"
#include "ocs_net/mdns_service.h"
#include "ocs_pipeline/basic/select_network_pipeline.h"
#include "ocs_pipeline/basic/system_pipeline.h"
#include "ocs_pipeline/httpserver/ap_network_handler.h"
#include "ocs_pipeline/httpserver/http_pipeline.h"
#include "ocs_pipeline/httpserver/sta_network_handler.h"
#include "ocs_pipeline/httpserver/time_pipeline.h"
#include "ocs_pipeline/httpserver/web_gui_pipeline.h"
#include "ocs_pipeline/jsonfmt/data_pipeline.h"

#ifdef CONFIG_BONSAI_FIRMWARE_CONSOLE_ENABLE
#include "ocs_pipeline/jsonfmt/console_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_CONSOLE_ENABLE

#if defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE)               \
    || defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)
#include "main/ds18b20_pipeline.h"
#endif // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE) ||
       // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)

#if defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE)                            \
    || defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE)
#include "main/soil_pipeline.h"
#endif // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE) ||
       // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE)

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
#include "main/sht41_pipeline.h"
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#include "main/control_pipeline.h"

namespace ocs {
namespace bonsai {

class ProjectPipeline : private system::ISuspendHandler, private core::NonCopyable<> {
public:
    //! Initialize.
    ProjectPipeline();

    //! Start the soil control system.
    status::StatusCode start();

private:
    status::StatusCode handle_suspend() override;
    status::StatusCode handle_resume() override;

    static constexpr const char* mdns_config_storage_id_ = "mdns_config";

    std::unique_ptr<pipeline::basic::SystemPipeline> system_pipeline_;
    std::unique_ptr<pipeline::jsonfmt::DataPipeline> json_data_pipeline_;

#ifdef CONFIG_BONSAI_FIRMWARE_CONSOLE_ENABLE
    std::unique_ptr<pipeline::jsonfmt::ConsolePipeline> console_pipeline_;
#endif // CONFIG_BONSAI_FIRMWARE_CONSOLE_ENABLE

    std::unique_ptr<net::FanoutNetworkHandler> fanout_network_handler_;

    storage::StorageBuilder::IStoragePtr mdns_config_storage_;
    std::unique_ptr<net::MdnsConfig> mdns_config_;
    std::unique_ptr<net::MdnsService> http_mdns_service_;
    std::unique_ptr<net::BasicMdnsServer> mdns_server_;

    std::unique_ptr<pipeline::httpserver::HttpPipeline> http_pipeline_;
    std::unique_ptr<pipeline::httpserver::TimePipeline> time_pipeline_;

    std::unique_ptr<pipeline::basic::SelectNetworkPipeline> network_pipeline_;
    std::unique_ptr<fmt::json::IFormatter> ap_network_formatter_;
    std::unique_ptr<pipeline::httpserver::ApNetworkHandler> ap_network_handler_;
    std::unique_ptr<fmt::json::IFormatter> sta_network_formatter_;
    std::unique_ptr<pipeline::httpserver::StaNetworkHandler> sta_network_handler_;

    std::unique_ptr<io::adc::IStore> adc_store_;
    std::unique_ptr<io::i2c::MasterStorePipeline> i2c_master_store_pipeline_;

    std::unique_ptr<ControlPipeline> control_pipeline_;

#ifdef CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE
    std::unique_ptr<SHT41Pipeline> sht41_pipeline_;
#endif // CONFIG_BONSAI_FIRMWARE_SENSOR_SHT41_ENABLE

#if defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE)                            \
    || defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE)
    std::unique_ptr<SoilPipeline> soil_pipeline_;
#endif // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_ENABLE) ||
       // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_SOIL_ANALOG_RELAY_ENABLE)

#if defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE)               \
    || defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)
    std::unique_ptr<DS18B20Pipeline> ds18b20_pipeline_;
#endif // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_SOIL_TEMPERATURE_ENABLE) ||
       // defined(CONFIG_BONSAI_FIRMWARE_SENSOR_DS18B20_OUTSIDE_TEMPERATURE_ENABLE)

    std::unique_ptr<pipeline::httpserver::WebGuiPipeline> web_gui_pipeline_;
};

} // namespace bonsai
} // namespace ocs
