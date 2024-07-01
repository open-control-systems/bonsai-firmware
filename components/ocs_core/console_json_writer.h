#pragma once

#include "ocs_core/ijson_writer.h"
#include "ocs_core/noncopyable.h"

namespace ocs {
namespace core {

class ConsoleJSONWriter : public IJSONWriter, public NonCopyable<> {
public:
    //! Write soil moisture data to the console.
    status::StatusCode write(const cJSONSharedBuilder::Ptr& json) override;
};

} // namespace core
} // namespace ocs
