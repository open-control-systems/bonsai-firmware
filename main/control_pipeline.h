/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include "fanout_telemetry_writer.h"
#include "gpio_config.h"
#include "http_command_handler.h"
#include "ocs_core/iclock.h"
#include "ocs_core/noncopyable.h"
#include "ocs_iot/counter_json_formatter.h"
#include "ocs_iot/default_http_handler.h"
#include "ocs_iot/system_counter_pipeline.h"
#include "ocs_net/http_server.h"
#include "ocs_net/mdns_provider.h"
#include "ocs_net/wifi_network.h"
#include "ocs_storage/flash_initializer.h"
#include "ocs_storage/storage_builder.h"
#include "ocs_system/fanout_reboot_handler.h"
#include "ocs_system/irebooter.h"
#include "registration_formatter.h"
#include "soil_moisture_monitor.h"
#include "telemetry_formatter.h"
#include "telemetry_holder.h"

namespace ocs {
namespace app {

class ControlPipeline : public net::INetworkHandler, public core::NonCopyable<> {
public:
    //! Initialize.
    ControlPipeline();

    //! Start providing telemetry data via HTTP server.
    void handle_connected() override;

    //! Stop providing telemetry data via HTTP server.
    void handle_disconnected() override;

    //! Start the soil control system.
    void start();

private:
    bool try_start_wifi_();
    void try_start_mdns_();

    using HTTPRegistrationHandler = iot::DefaultHTTPHandler<256>;
    using HTTPTelemetryHandler = iot::DefaultHTTPHandler<256>;

    std::unique_ptr<ITelemetryReader> adc_reader_;
    std::unique_ptr<ITelemetryReader> moisture_reader_;

    std::unique_ptr<storage::FlashInitializer> flash_initializer_;
    std::unique_ptr<net::WiFiNetwork> wifi_network_;
    std::unique_ptr<net::HTTPServer> http_server_;
    std::unique_ptr<net::MDNSProvider> mdns_provider_;

    std::unique_ptr<core::IClock> default_clock_;

    std::unique_ptr<system::FanoutRebootHandler> fanout_reboot_handler_;
    std::unique_ptr<system::IRebooter> default_rebooter_;
    std::unique_ptr<system::IRebooter> delay_rebooter_;

    std::unique_ptr<FanoutTelemetryWriter> fanout_telemetry_writer_;

    std::unique_ptr<TelemetryHolder> telemetry_holder_;
    std::unique_ptr<TelemetryFormatter> telemetry_formatter_;
    std::unique_ptr<HTTPTelemetryHandler> http_telemetry_handler_;

    std::unique_ptr<ITelemetryWriter> console_telemetry_writer_;

    std::unique_ptr<GPIOConfig> gpio_config_;
    std::unique_ptr<SoilMoistureMonitor> soil_moisture_monitor_;

    std::unique_ptr<HTTPCommandHandler> http_command_handler_;

    std::unique_ptr<RegistrationFormatter> registration_formatter_;
    std::unique_ptr<HTTPRegistrationHandler> http_registration_handler_;

    std::unique_ptr<storage::StorageBuilder> storage_builder_;
    std::unique_ptr<iot::CounterJSONFormatter> counter_json_formatter_;
    std::unique_ptr<iot::SystemCounterPipeline> system_counter_pipeline_;
};

} // namespace app
} // namespace ocs
