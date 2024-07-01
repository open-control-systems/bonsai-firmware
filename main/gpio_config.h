#pragma once

#include "driver/gpio.h"

#include "ocs_core/noncopyable.h"

namespace ocs {
namespace app {

class GPIOConfig : public core::NonCopyable<> {
public:
    //! Initialize system wide GPIO configuration.
    GPIOConfig();

private:
    gpio_config_t config_ {};
};

} // namespace app
} // namespace ocs
