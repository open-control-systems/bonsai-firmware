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

#include "scs/control_pipeline.h"
#include "scs/data_pipeline.h"
#include "scs/http_pipeline.h"
#include "scs/system_pipeline.h"

namespace ocs {
namespace app {

class ProjectPipeline : public core::NonCopyable<> {
public:
    //! Initialize.
    ProjectPipeline();

    //! Start the soil control system.
    status::StatusCode start();

private:
    std::unique_ptr<SystemPipeline> system_pipeline_;
    std::unique_ptr<DataPipeline> data_pipeline_;
    std::unique_ptr<ControlPipeline> control_pipeline_;

    std::unique_ptr<HttpPipeline<256, 512>> http_pipeline_;
};

} // namespace app
} // namespace ocs
