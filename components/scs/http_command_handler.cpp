/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_log.h"
#include "esp_system.h"

#include "freertos/FreeRTOSConfig.h"

#include "ocs_iot/cjson_array_formatter.h"
#include "ocs_iot/cjson_builder.h"
#include "scs/http_command_handler.h"

namespace ocs {
namespace app {

namespace {

const char* log_tag = "http-command-handler";

} // namespace

HttpCommandHandler::HttpCommandHandler(system::IRebooter& rebooter,
                                       net::HttpServer& server,
                                       SoilMoistureMonitor& monitor) {
    commands_response_.reset(new (std::nothrow) JsonFormatter());
    configASSERT(commands_response_);

    format_commands_response_();
    register_routes_(rebooter, server, monitor);
}

void HttpCommandHandler::format_commands_response_() {
    auto json = iot::CjsonUniqueBuilder::make_json();
    configASSERT(json);

    auto array = cJSON_AddArrayToObject(json.get(), "commands");
    configASSERT(array);

    iot::CjsonArrayFormatter formatter(array);

    configASSERT(formatter.append_string_ref("reboot"));
    configASSERT(formatter.append_string_ref("reload"));

    commands_response_->format(json.get());
}

void HttpCommandHandler::register_routes_(system::IRebooter& rebooter,
                                          net::HttpServer& server,
                                          SoilMoistureMonitor& monitor) {
    server.add_GET("/commands/reboot", [&rebooter](httpd_req_t* req) {
        const auto err = httpd_resp_send(req, "Rebooting...", HTTPD_RESP_USE_STRLEN);
        if (err != ESP_OK) {
            return status::StatusCode::Error;
        }

        rebooter.reboot();

        return status::StatusCode::OK;
    });
    server.add_GET("/commands/reload", [&monitor](httpd_req_t* req) {
        const auto err = httpd_resp_send(req, "Reloading...", HTTPD_RESP_USE_STRLEN);
        if (err != ESP_OK) {
            return status::StatusCode::Error;
        }

        ESP_LOGI(log_tag, "Reloading...");

        monitor.reload();

        return status::StatusCode::OK;
    });
    server.add_GET("/commands", [this](httpd_req_t* req) {
        const auto err =
            httpd_resp_send(req, commands_response_->c_str(), HTTPD_RESP_USE_STRLEN);
        if (err != ESP_OK) {
            return status::StatusCode::Error;
        }

        return status::StatusCode::OK;
    });
}

} // namespace app
} // namespace ocs
