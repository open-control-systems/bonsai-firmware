#pragma once

#include "ijson_writer.h"
#include "noncopyable.h"

class ConsoleJSONWriter : public IJSONWriter, public NonCopyable<> {
public:
    //! Write soil moisture data to the console.
    StatusCode write(const cJSONSharedBuilder::Ptr& json) override;
};
