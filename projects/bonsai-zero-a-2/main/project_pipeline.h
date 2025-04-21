/*
 * Copyright (c) 2025, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include "ocs_core/iclock.h"
#include "ocs_core/noncopyable.h"
#include "ocs_fmt/json/fanout_formatter.h"
#include "ocs_http/irouter.h"
#include "ocs_http/iserver.h"
#include "ocs_io/adc/iconverter.h"
#include "ocs_io/adc/istore.h"
#include "ocs_net/basic_mdns_server.h"
#include "ocs_net/fanout_network_handler.h"
#include "ocs_net/mdns_config.h"
#include "ocs_net/mdns_service.h"
#include "ocs_pipeline/basic/select_network_pipeline.h"
#include "ocs_pipeline/basic/system_pipeline.h"
#include "ocs_pipeline/httpserver/analog_config_store_handler.h"
#include "ocs_pipeline/httpserver/ap_network_handler.h"
#include "ocs_pipeline/httpserver/http_pipeline.h"
#include "ocs_pipeline/httpserver/sta_network_handler.h"
#include "ocs_pipeline/httpserver/time_pipeline.h"
#include "ocs_pipeline/httpserver/web_gui_pipeline.h"
#include "ocs_pipeline/jsonfmt/data_pipeline.h"
#include "ocs_scheduler/itask_scheduler.h"
#include "ocs_sensor/analog_config_store.h"
#include "ocs_sensor/soil/analog_sensor_pipeline.h"
#include "ocs_storage/storage_builder.h"
#include "ocs_system/fanout_reboot_handler.h"
#include "ocs_system/fanout_suspender.h"
#include "ocs_system/platform_builder.h"

namespace ocs {
namespace bonsai {

class ProjectPipeline : private system::ISuspendHandler,
                        private fmt::json::IFormatter,
                        private core::NonCopyable<> {
public:
    //! Initialize.
    ProjectPipeline();

    //! Start the soil control system.
    status::StatusCode start();

private:
    status::StatusCode handle_suspend() override;
    status::StatusCode handle_resume() override;
    status::StatusCode format(cJSON* json) override;

    static constexpr const char* mdns_config_storage_id_ = "mdns_config";
    static constexpr const char* analog_config_storage_id_ = "analog_config";

    system::PlatformBuilder::IRtDelayerPtr rt_delayer_;
    std::unique_ptr<system::FanoutSuspender> fanout_suspender_;

    std::unique_ptr<pipeline::basic::SystemPipeline> system_pipeline_;
    std::unique_ptr<pipeline::jsonfmt::DataPipeline> json_data_pipeline_;

    std::unique_ptr<net::FanoutNetworkHandler> fanout_network_handler_;

    storage::StorageBuilder::IStoragePtr mdns_config_storage_;
    std::unique_ptr<net::MdnsConfig> mdns_config_;
    std::unique_ptr<net::MdnsService> http_mdns_service_;
    std::unique_ptr<net::BasicMdnsServer> mdns_server_;

    std::unique_ptr<http::IRouter> http_router_;
    std::unique_ptr<http::IServer> http_server_;
    std::unique_ptr<pipeline::httpserver::HttpPipeline> http_pipeline_;
    std::unique_ptr<pipeline::httpserver::TimePipeline> time_pipeline_;

    std::unique_ptr<pipeline::basic::SelectNetworkPipeline> network_pipeline_;
    std::unique_ptr<fmt::json::IFormatter> ap_network_formatter_;
    std::unique_ptr<pipeline::httpserver::ApNetworkHandler> ap_network_handler_;
    std::unique_ptr<fmt::json::IFormatter> sta_network_formatter_;
    std::unique_ptr<pipeline::httpserver::StaNetworkHandler> sta_network_handler_;

    std::unique_ptr<io::adc::IStore> adc_store_;
    std::unique_ptr<io::adc::IConverter> adc_converter_;

    storage::StorageBuilder::IStoragePtr analog_config_storage_;
    std::unique_ptr<sensor::AnalogConfigStore> analog_config_store_;
    std::unique_ptr<pipeline::httpserver::AnalogConfigStoreHandler>
        analog_config_store_handler_;

    static constexpr const char* soil_sensor_id_0_ = "soil_a0";
    std::unique_ptr<sensor::AnalogConfig> soil_sensor_config_0_;
    std::unique_ptr<sensor::soil::AnalogSensorPipeline> soil_sensor_pipeline_0_;

    static constexpr const char* soil_sensor_id_1_ = "soil_a1";
    std::unique_ptr<sensor::AnalogConfig> soil_sensor_config_1_;
    std::unique_ptr<sensor::soil::AnalogSensorPipeline> soil_sensor_pipeline_1_;

    std::unique_ptr<pipeline::httpserver::WebGuiPipeline> web_gui_pipeline_;
};

} // namespace bonsai
} // namespace ocs
