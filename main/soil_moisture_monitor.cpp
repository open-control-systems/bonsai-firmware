#include "soil_moisture_monitor.h"
#include "ocs_status/code.h"
#include "ocs_status/code_to_str.h"

namespace ocs {
namespace app {

SoilMoistureMonitor::SoilMoistureMonitor(SoilMoistureMonitor::Params params,
                                         core::IJSONReader& reader,
                                         core::IJSONWriter& writer)
    : params_(params)
    , reader_(reader)
    , writer_(writer) {
}

void SoilMoistureMonitor::start() {
    while (true) {
        relay_turn_on_();
        vTaskDelay(params_.power_up_delay_interval);

        auto data = core::cJSONSharedBuilder::make_json();

        auto status = reader_.read(data);
        if (status == status::StatusCode::OK) {
            status = writer_.write(data);
            if (status != status::StatusCode::OK) {
                fprintf(stderr, "Failed to write data: err=%s\n",
                        status::code_to_str(status));
            }
        } else {
            fprintf(stderr, "Failed to read data: err=%s\n", status::code_to_str(status));
        }

        relay_turn_off_();
        vTaskDelay(params_.read_delay_interval);
    }
}

void SoilMoistureMonitor::relay_turn_on_() {
    gpio_set_level(params_.relay_gpio, true);
}

void SoilMoistureMonitor::relay_turn_off_() {
    gpio_set_level(params_.relay_gpio, false);
}

} // namespace app
} // namespace ocs
