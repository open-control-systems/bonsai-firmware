/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include "ocs_core/iclock.h"
#include "ocs_core/noncopyable.h"
#include "ocs_iot/counter_json_formatter.h"
#include "ocs_iot/default_http_handler.h"
#include "ocs_iot/http_server_pipeline.h"
#include "ocs_iot/system_counter_pipeline.h"
#include "ocs_storage/flash_initializer.h"
#include "ocs_storage/storage_builder.h"
#include "ocs_system/fanout_reboot_handler.h"
#include "ocs_system/irebooter.h"
#include "scs/fanout_telemetry_writer.h"
#include "scs/gpio_config.h"
#include "scs/http_command_handler.h"
#include "scs/registration_formatter.h"
#include "scs/soil_moisture_monitor.h"
#include "scs/telemetry_formatter.h"
#include "scs/telemetry_holder.h"

namespace ocs {
namespace app {

class ControlPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    ControlPipeline();

    //! Start the soil control system.
    void start();

private:
    void register_mdns_endpoints_();

    using HTTPRegistrationHandler = iot::DefaultHTTPHandler<256>;
    using HTTPTelemetryHandler = iot::DefaultHTTPHandler<256>;

    std::unique_ptr<ITelemetryReader> adc_reader_;
    std::unique_ptr<ITelemetryReader> moisture_reader_;

    std::unique_ptr<storage::FlashInitializer> flash_initializer_;
    std::unique_ptr<iot::HTTPServerPipeline> http_server_pipeline_;

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
