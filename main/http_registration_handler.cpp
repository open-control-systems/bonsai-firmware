/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_log.h"

#include "http_registration_handler.h"
#include "ocs_iot/cjson_builder.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "http-registration-handler";

} // namespace

HTTPRegistrationHandler::HTTPRegistrationHandler(net::HTTPServer& server,
                                                 RegistrationFormatter& formatter) {
    json_formatter_.reset(new (std::nothrow) JSONFormatter(formatter));

    server.add_GET("/registration", [this](httpd_req_t* req) {
        auto json = iot::cJSONUniqueBuilder::make_json();
        json_formatter_->format(json.get());

        const auto err =
            httpd_resp_send(req, json_formatter_->c_str(), HTTPD_RESP_USE_STRLEN);
        if (err != ESP_OK) {
            ESP_LOGE(log_tag, "httpd_resp_send(): %s", esp_err_to_name(err));
            return status::StatusCode::Error;
        }

        return status::StatusCode::OK;
    });
}

} // namespace app
} // namespace ocs
