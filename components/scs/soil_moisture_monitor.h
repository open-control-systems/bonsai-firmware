/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "driver/gpio.h"

#include "ocs_core/noncopyable.h"
#include "ocs_scheduler/itask.h"
#include "scs/itelemetry_reader.h"
#include "scs/itelemetry_writer.h"

namespace ocs {
namespace app {

class SoilMoistureMonitor : public scheduler::ITask, public core::NonCopyable<> {
public:
    struct Params {
        //! Interval to wait after the control system is powered on.
        TickType_t power_on_delay_interval { pdMS_TO_TICKS(0) };

        //! Relay GPIO.
        gpio_num_t relay_gpio { GPIO_NUM_NC };
    };

    //! Initialize.
    SoilMoistureMonitor(Params params,
                        ITelemetryReader& reader,
                        ITelemetryWriter& writer);

    //! Monitor soil moisture data.
    status::StatusCode run() override;

private:
    status::StatusCode run_();
    void relay_turn_on_();
    void relay_turn_off_();

    const Params params_;

    ITelemetryReader& reader_;
    ITelemetryWriter& writer_;
};

} // namespace app
} // namespace ocs
