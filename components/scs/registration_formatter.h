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

namespace ocs {
namespace scs {

class RegistrationFormatter : public iot::IJsonFormatter, public core::NonCopyable<> {
public:
    //! Initialize.
    RegistrationFormatter();

    //! Format the underlying data into @p json.
    void format(cJSON* json) override;

    iot::FanoutJsonFormatter& get_fanout_formatter();

private:
    std::unique_ptr<iot::FanoutJsonFormatter> fanout_formatter_;
    std::unique_ptr<iot::VersionJsonFormatter> version_formatter_;
};

} // namespace scs
} // namespace ocs
