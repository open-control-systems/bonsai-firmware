#include <cstdio>
#include <cstring>

#include "driver/gpio.h"

#include "adc_reader.h"
#include "console_json_writer.h"
#include "gpio_config.h"
#include "ijson_writer.h"
#include "soil_moisture_monitor.h"
#include "yl69_moisture_reader.h"

namespace {

using IJSONWriterPtr = std::unique_ptr<IJSONWriter>;

IJSONWriterPtr select_json_writer() {
    IJSONWriterPtr ret;

    ret = std::make_unique<ConsoleJSONWriter>();

    return ret;
}

} // namespace

extern "C" void app_main(void) {
    ADCReader adc_reader(ADCReader::Params {
        // GPIO 34 -> ADC CHANNEL 6
        .channel = ADC_CHANNEL_6,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_10,
    });

    YL69MoistureReader moisture_reader(800, adc_reader);
    auto moisture_writer = select_json_writer();

    GPIOConfig config;

    SoilMoistureMonitor monitor(
        SoilMoistureMonitor::Params {
            .power_up_delay_interval = pdMS_TO_TICKS(1000 * 1),
            .read_delay_interval = pdMS_TO_TICKS(1000 * 60 * 30),
            .relay_gpio = GPIO_NUM_26,
        },
        moisture_reader, *moisture_writer);

    monitor.start();
}
