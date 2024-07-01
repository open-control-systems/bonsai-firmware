#pragma once

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

#include "ocs_core/ijson_reader.h"
#include "ocs_core/noncopyable.h"

namespace ocs {
namespace app {

class ADCReader : public core::IJSONReader, public core::NonCopyable<> {
public:
    struct Params {
        adc_channel_t channel { ADC_CHANNEL_0 };
        adc_atten_t atten { ADC_ATTEN_DB_0 };
        adc_bitwidth_t bitwidth { ADC_BITWIDTH_DEFAULT };
    };

    //! Initialize ESP specific ADC instances.
    explicit ADCReader(Params);

    //! De-initialize ESP specific ADC instances.
    ~ADCReader();

    //! Read raw value for configured channel.
    status::StatusCode read(core::cJSONSharedBuilder::Ptr& json) override;

private:
    const Params params_;

    adc_oneshot_chan_cfg_t config_;
    adc_oneshot_unit_init_cfg_t unit_config_;
    adc_cali_line_fitting_config_t calibration_config_;

    adc_oneshot_unit_handle_t unit_handle_ { nullptr };
    adc_cali_handle_t calibration_handle_ { nullptr };
};

} // namespace app
} // namespace ocs
