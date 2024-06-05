#pragma once

#include "driver/gpio.h"

#include "noncopyable.h"

class GPIOConfig : public NonCopyable<> {
public:
    //! Initialize system wide GPIO configuration.
    GPIOConfig();

private:
    gpio_config_t config_ {};
};
