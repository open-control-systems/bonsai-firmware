/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "esp_system.h"

#include "http_command_handler.h"

namespace ocs {
namespace app {

HTTPCommandHandler::HTTPCommandHandler(net::HTTPServer& server,
                                       SoilMoistureMonitor& monitor) {
    server.add_GET("/commands/reboot", [](httpd_req_t* req) {
        const auto err = httpd_resp_send(req, "Rebooting...", HTTPD_RESP_USE_STRLEN);
        if (err != ESP_OK) {
            return status::StatusCode::Error;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
        esp_restart();

        return status::StatusCode::OK;
    });
    server.add_GET("/commands/reload", [&monitor](httpd_req_t* req) {
        const auto err = httpd_resp_send(req, "Reloading...", HTTPD_RESP_USE_STRLEN);
        if (err != ESP_OK) {
            return status::StatusCode::Error;
        }

        monitor.reload();

        return status::StatusCode::OK;
    });
}

} // namespace app
} // namespace ocs
