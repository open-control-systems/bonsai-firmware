#pragma once

#include "ocs_core/cjson_builder.h"
#include "ocs_status/code.h"

namespace ocs {
namespace core {

class IJSONReader {
public:
    //! Destroy.
    virtual ~IJSONReader() = default;

    //! Read soil moisture characteristics into @p json.
    [[nodiscard]] virtual status::StatusCode read(cJSONSharedBuilder::Ptr& json) = 0;
};

} // namespace core
} // namespace ocs
