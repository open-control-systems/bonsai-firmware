#include <cstring>

#include "adc_reader.h"
#include "ocs_core/cjson_object_formatter.h"
#include "types.h"

namespace ocs {
namespace app {

ADCReader::ADCReader(ADCReader::Params params)
    : params_(params) {
    // Unit configuration.
    memset(&unit_config_, 0, sizeof(unit_config_));
    unit_config_.unit_id = ADC_UNIT_1;
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config_, &unit_handle_));

    // ADC1 configuration.
    memset(&config_, 0, sizeof(config_));
    config_.bitwidth = params_.bitwidth;
    config_.atten = params_.atten;
    ESP_ERROR_CHECK(adc_oneshot_config_channel(unit_handle_, params_.channel, &config_));

    // ADC1 calibration configuration.
    memset(&calibration_config_, 0, sizeof(calibration_config_));
    calibration_config_.unit_id = ADC_UNIT_1;
    calibration_config_.atten = params_.atten;
    calibration_config_.bitwidth = params_.bitwidth;
    ESP_ERROR_CHECK(
        adc_cali_create_scheme_line_fitting(&calibration_config_, &calibration_handle_));
}

ADCReader::~ADCReader() {
    ESP_ERROR_CHECK(adc_oneshot_del_unit(unit_handle_));
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(calibration_handle_));
}

status::StatusCode ADCReader::read(core::cJSONSharedBuilder::Ptr& json) {
    int raw = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(unit_handle_, params_.channel, &raw));

    core::cJSONObjectFormatter formatter(json.get());

    formatter.add_number_cs(
        soil_moisture_characteristic_to_str(SoilMoistureCharacteristic::Raw), raw);

    int voltage = 0;
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(calibration_handle_, raw, &voltage));

    formatter.add_number_cs(
        soil_moisture_characteristic_to_str(SoilMoistureCharacteristic::Voltage),
        voltage);

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
