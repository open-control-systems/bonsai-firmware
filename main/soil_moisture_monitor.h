#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "driver/gpio.h"

#include "ocs_core/ijson_reader.h"
#include "ocs_core/ijson_writer.h"
#include "ocs_core/noncopyable.h"

namespace ocs {
namespace app {

class SoilMoistureMonitor : public core::NonCopyable<> {
public:
    struct Params {
        //! Wait for some time for a switching scheme to be powered on.
        TickType_t power_up_delay_interval { pdMS_TO_TICKS(0) };

        //! Too frequent reads may damage the sensor, read the corresponding datasheet.
        TickType_t read_delay_interval { pdMS_TO_TICKS(0) };

        //! Relay GPIO.
        gpio_num_t relay_gpio { GPIO_NUM_NC };
    };

    //! Initialize.
    SoilMoistureMonitor(Params params,
                        core::IJSONReader& reader,
                        core::IJSONWriter& writer);

    //! Start monitoring soil status.
    //!
    //! @remarks
    //!  - Blocking call.
    void start();

private:
    void relay_turn_on_();
    void relay_turn_off_();

    const Params params_;

    core::IJSONReader& reader_;
    core::IJSONWriter& writer_;
};

} // namespace app
} // namespace ocs
