#pragma once

#include "ijson_reader.h"
#include "noncopyable.h"

class YL69MoistureReader : public IJSONReader, public NonCopyable<> {
public:
    //! Initialize.
    //!
    //! @params
    //!  - @p threshold after reaching which to consider the soil is wet.
    //!  - @p reader from which to read raw data.
    YL69MoistureReader(int threshold, IJSONReader& reader);

    //! Convert raw data based on the provided threshold to the soil moisture status.
    StatusCode read(cJSONSharedBuilder::Ptr& data) override;

private:
    const int threshold_ { 0 };

    IJSONReader& reader_;
};
