/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_log.h"

#include "ocs_core/version.h"
#include "ocs_core/version_to_str.h"
#include "ocs_iot/network_json_formatter.h"
#include "registration_formatter.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "registration-formatter";

} // namespace

RegistrationFormatter::RegistrationFormatter(net::BasicNetwork& network) {
    fanout_formatter_.reset(new (std::nothrow) iot::FanoutJSONFormatter());

    network_formatter_.reset(new (std::nothrow) iot::NetworkJSONFormatter(network));
    fanout_formatter_->add(*network_formatter_);

    core::Version version;
    if (!version.parse(CONFIG_OCS_CORE_FW_VERSION)) {
        ESP_LOGE(log_tag, "failed to parse FW version: %s", CONFIG_OCS_CORE_FW_VERSION);
    }

    core::version_to_str version_str(version);

    version_formatter_.reset(new (std::nothrow) iot::VersionJSONFormatter());
    version_formatter_->add("version_scs", version_str.c_str());
    fanout_formatter_->add(*version_formatter_);
}

void RegistrationFormatter::format(cJSON* json) {
    fanout_formatter_->format(json);
}

} // namespace app
} // namespace ocs
