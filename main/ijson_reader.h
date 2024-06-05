#pragma once

#include "cjson_builder.h"
#include "status_code.h"

class IJSONReader {
public:
    //! Destroy.
    virtual ~IJSONReader() = default;

    //! Read soil moisture characteristics into @p json.
    [[nodiscard]] virtual StatusCode read(cJSONSharedBuilder::Ptr& json) = 0;
};
