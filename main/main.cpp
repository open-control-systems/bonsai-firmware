#include <cstdio>
#include <cstring>

#include "driver/gpio.h"

#include "adc_reader.h"
#include "console_telemetry_writer.h"
#include "gpio_config.h"
#include "itelemetry_writer.h"
#include "soil_moisture_monitor.h"
#include "yl69_moisture_reader.h"

using namespace ocs;
using namespace ocs::app;

namespace {

using ITelemetryWriterPtr = std::unique_ptr<ITelemetryWriter>;

ITelemetryWriterPtr select_json_writer() {
    ITelemetryWriterPtr ret;

    ret = std::make_unique<ConsoleTelemetryWriter>();

    return ret;
}

} // namespace

extern "C" void app_main(void) {
    ADCReader adc_reader(ADCReader::Params {
        // GPIO 34 -> ADC CHANNEL 6
        .channel = static_cast<adc_channel_t>(CONFIG_SMC_SENSOR_ADC_CHANNEL),
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    });

    YL69MoistureReader moisture_reader(CONFIG_OCS_MOISTURE_SENSOR_THRESHOLD, adc_reader);
    auto moisture_writer = select_json_writer();
    YL69MoistureReader moisture_reader(CONFIG_SMC_SENSOR_THRESHOLD, adc_reader);

    GPIOConfig config;

    SoilMoistureMonitor monitor(
        SoilMoistureMonitor::Params {
            .power_up_delay_interval =
                pdMS_TO_TICKS(1000 * CONFIG_SMC_POWER_ON_DELAY_INTERVAL),
            .read_delay_interval = pdMS_TO_TICKS(1000 * CONFIG_SMC_READ_INTERVAL),
            .relay_gpio = static_cast<gpio_num_t>(CONFIG_SMC_RELAY_GPIO),
        },
        moisture_reader, *moisture_writer);

    monitor.start();
}
