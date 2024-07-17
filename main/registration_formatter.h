/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include "ocs_core/noncopyable.h"
#include "ocs_iot/fanout_json_formatter.h"
#include "ocs_iot/ijson_formatter.h"
#include "ocs_iot/version_json_formatter.h"
#include "ocs_net/basic_network.h"

namespace ocs {
namespace app {

class RegistrationFormatter : public iot::IJSONFormatter, public core::NonCopyable<> {
public:
    //! Initialize.
    //!
    //! @params
    //!  - @p network to read the network data.
    explicit RegistrationFormatter(net::BasicNetwork& network);

    //! Format the underlying data into @p json.
    void format(cJSON* json) override;

private:
    std::unique_ptr<iot::FanoutJSONFormatter> fanout_formatter_;
    std::unique_ptr<iot::IJSONFormatter> network_formatter_;
    std::unique_ptr<iot::VersionJSONFormatter> version_formatter_;
};

} // namespace app
} // namespace ocs
