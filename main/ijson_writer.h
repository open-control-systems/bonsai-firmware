#pragma once

#include <memory>

#include "cjson_builder.h"
#include "status_code.h"

class IJSONWriter {
public:
    //! Destroy.
    virtual ~IJSONWriter() = default;

    //! Write various soil moisture characteristics.
    [[nodiscard]] virtual StatusCode write(const cJSONSharedBuilder::Ptr& json) = 0;
};
