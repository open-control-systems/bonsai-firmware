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
#include "ocs_iot/default_json_formatter.h"
#include "ocs_net/http_server.h"
#include "registration_formatter.h"

namespace ocs {
namespace app {

class HTTPRegistrationHandler : public core::NonCopyable<> {
public:
    //! Initialize.
    //!
    //! @params
    //!  - @p server to register the HTTP endpoint for the registration data.
    //!  - @p formatter to format the registration data.
    HTTPRegistrationHandler(net::HTTPServer& server, RegistrationFormatter& formatter);

private:
    using JSONFormatter = iot::DefaultJSONFormatter<256>;

    std::unique_ptr<JSONFormatter> json_formatter_;
};

} // namespace app
} // namespace ocs
