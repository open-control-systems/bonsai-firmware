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
#include "ocs_iot/console_json_pipeline.h"
#include "ocs_iot/http_pipeline.h"
#include "ocs_iot/json_data_pipeline.h"
#include "ocs_iot/system_pipeline.h"

#include "scs/control_pipeline.h"

namespace ocs {
namespace scs {

class ProjectPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    ProjectPipeline();

    //! Start the soil control system.
    status::StatusCode start();

private:
    std::unique_ptr<iot::SystemPipeline> system_pipeline_;
    std::unique_ptr<iot::JsonDataPipeline> json_data_pipeline_;

    using ConsolePipeline = iot::ConsoleJsonPipeline<512, 256>;
    std::unique_ptr<ConsolePipeline> console_pipeline_;

    std::unique_ptr<ControlPipeline> control_pipeline_;

    using HttpPipeline = iot::HttpPipeline<512, 256, 256>;
    std::unique_ptr<HttpPipeline> http_pipeline_;
};

} // namespace scs
} // namespace ocs
