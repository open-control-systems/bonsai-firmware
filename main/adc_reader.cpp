/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>

#include "adc_reader.h"
#include "telemetry.h"

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

status::StatusCode ADCReader::read(Telemetry& telemetry) {
    ESP_ERROR_CHECK(adc_oneshot_read(unit_handle_, params_.channel, &telemetry.raw));

    ESP_ERROR_CHECK(
        adc_cali_raw_to_voltage(calibration_handle_, telemetry.raw, &telemetry.voltage));

    return status::StatusCode::OK;
}

} // namespace app
} // namespace ocs
