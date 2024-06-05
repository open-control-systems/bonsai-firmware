#include "soil_moisture_monitor.h"
#include "status_code.h"
#include "status_code_to_str.h"

SoilMoistureMonitor::SoilMoistureMonitor(SoilMoistureMonitor::Params params,
                                         IJSONReader& reader,
                                         IJSONWriter& writer)
    : params_(params)
    , reader_(reader)
    , writer_(writer) {
}

void SoilMoistureMonitor::start() {
    while (true) {
        relay_turn_on_();
        vTaskDelay(params_.power_up_delay_interval);

        auto data = cJSONSharedBuilder::make_json();

        auto status = reader_.read(data);
        if (status == StatusCode::OK) {
            status = writer_.write(data);
            if (status != StatusCode::OK) {
                fprintf(stderr, "Failed to write data: err=%s\n",
                        status_code_to_str(status));
            }
        } else {
            fprintf(stderr, "Failed to read data: err=%s\n", status_code_to_str(status));
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
