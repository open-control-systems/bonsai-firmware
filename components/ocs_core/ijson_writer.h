#pragma once

#include <memory>

#include "ocs_core/cjson_builder.h"
#include "ocs_status/code.h"

namespace ocs {
namespace core {

class IJSONWriter {
public:
    //! Destroy.
    virtual ~IJSONWriter() = default;

    //! Write various soil moisture characteristics.
    [[nodiscard]] virtual status::StatusCode
    write(const cJSONSharedBuilder::Ptr& json) = 0;
};

} // namespace core
} // namespace ocs
