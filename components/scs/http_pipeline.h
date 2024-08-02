/*
 * Copyright (c) 2024, Open Control Systems authors
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ocs_core/noncopyable.h"
#include "ocs_iot/default_http_handler.h"
#include "ocs_iot/fanout_json_formatter.h"
#include "ocs_iot/http_server_pipeline.h"
#include "ocs_iot/network_json_formatter.h"
#include "ocs_scheduler/itask.h"
#include "ocs_status/code_to_str.h"

#include "scs/http_command_handler.h"

namespace ocs {
namespace app {

template <unsigned RegistrationSize, unsigned TelemetrySize>
class HttpPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    HttpPipeline(scheduler::ITask& reboot_task,
                 scheduler::ITask& control_task,
                 iot::IJsonFormatter& telemetry_formatter,
                 iot::FanoutJsonFormatter& registration_formatter) {
        http_server_pipeline_.reset(new (std::nothrow) iot::HttpServerPipeline());
        configASSERT(http_server_pipeline_);

        http_telemetry_handler_.reset(new (std::nothrow) HttpTelemetryHandler(
            http_server_pipeline_->server(), telemetry_formatter, "/telemetry",
            "http-telemetry-handler"));
        configASSERT(http_telemetry_handler_);

        http_registration_handler_.reset(new (std::nothrow) HttpRegistrationHandler(
            http_server_pipeline_->server(), registration_formatter, "/registration",
            "http-registration-handler"));
        configASSERT(http_registration_handler_);

        http_command_handler_.reset(new (std::nothrow) HttpCommandHandler(
            http_server_pipeline_->server(), reboot_task, control_task));
        configASSERT(http_command_handler_);

        network_formatter_.reset(new (std::nothrow) iot::NetworkJsonFormatter(
            http_server_pipeline_->network()));
        configASSERT(network_formatter_);

        registration_formatter.add(*network_formatter_);
    }

    //! Start the pipeline.
    status::StatusCode start() {
        auto code = http_server_pipeline_->start();
        if (code != status::StatusCode::OK) {
            ESP_LOGE(log_tag_, "failed to start HTTP server pipeline: code=%s",
                     status::code_to_str(code));

            return code;
        }

        code = register_mdns_endpoints_();
        if (code != status::StatusCode::OK) {
            ESP_LOGE(log_tag_, "failed to register mDNS endpoints: code=%s",
                     status::code_to_str(code));

            return code;
        }

        return status::StatusCode::OK;
    }

private:
    status::StatusCode register_mdns_endpoints_() {
        return http_server_pipeline_->mdns().add_service_txt_records(
            "_http", "_tcp",
            net::MdnsProvider::TxtRecordList {
                {
                    "telemetry",
                    "/telemetry",
                },
                {
                    "registration",
                    "/registration",
                },
                {
                    "command_reboot",
                    "/commands/reboot",
                },
                {
                    "command_reload",
                    "/commands/reload",
                },
                {
                    "commands",
                    "/commands",
                },
            });
    }

    using HttpRegistrationHandler = iot::DefaultHttpHandler<RegistrationSize>;
    using HttpTelemetryHandler = iot::DefaultHttpHandler<TelemetrySize>;

    static const constexpr char* log_tag_ = "http-pipeline";

    std::unique_ptr<iot::HttpServerPipeline> http_server_pipeline_;
    std::unique_ptr<HttpTelemetryHandler> http_telemetry_handler_;
    std::unique_ptr<HttpRegistrationHandler> http_registration_handler_;
    std::unique_ptr<HttpCommandHandler> http_command_handler_;
    std::unique_ptr<iot::IJsonFormatter> network_formatter_;
};

} // namespace app
} // namespace ocs
