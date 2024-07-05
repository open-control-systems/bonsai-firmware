#pragma once

#include <memory>

#include "fanout_telemetry_writer.h"
#include "gpio_config.h"
#include "http_server.h"
#include "ocs_core/noncopyable.h"
#include "ocs_net/mdns_provider.h"
#include "ocs_net/wifi_network.h"
#include "ocs_storage/flash_storage.h"
#include "soil_moisture_monitor.h"

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
    std::unique_ptr<ITelemetryReader> adc_reader_;
    std::unique_ptr<ITelemetryReader> moisture_reader_;

    std::unique_ptr<ITelemetryWriter> console_telemetry_writer_;
    std::unique_ptr<FanoutTelemetryWriter> fanout_telemetry_writer_;

    std::unique_ptr<storage::FlashStorage> flash_storage_;
    std::unique_ptr<net::WiFiNetwork> wifi_network_;
    std::unique_ptr<HTTPServer> http_server_;
    std::unique_ptr<net::MDNSProvider> mdns_provider_;

    std::unique_ptr<GPIOConfig> gpio_config_;
    std::unique_ptr<SoilMoistureMonitor> soil_moisture_monitor_;
};

} // namespace app
} // namespace ocs
