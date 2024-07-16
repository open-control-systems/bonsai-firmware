/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "registration_formatter.h"
#include "ocs_iot/network_json_formatter.h"

namespace ocs {
namespace app {

RegistrationFormatter::RegistrationFormatter(net::BasicNetwork& network) {
    network_formatter_.reset(new (std::nothrow) iot::NetworkJSONFormatter(network));
}

void RegistrationFormatter::format(cJSON* json) {
    network_formatter_->format(json);
}

} // namespace app
} // namespace ocs
