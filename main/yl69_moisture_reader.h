#pragma once

#include "ocs_core/ijson_reader.h"
#include "ocs_core/noncopyable.h"

namespace ocs {
namespace app {

class YL69MoistureReader : public core::IJSONReader, public core::NonCopyable<> {
public:
    //! Initialize.
    //!
    //! @params
    //!  - @p threshold after reaching which to consider the soil is wet.
    //!  - @p reader from which to read raw data.
    YL69MoistureReader(int threshold, core::IJSONReader& reader);

    //! Convert raw data based on the provided threshold to the soil moisture status.
    status::StatusCode read(core::cJSONSharedBuilder::Ptr& data) override;

private:
    const int threshold_ { 0 };

    IJSONReader& reader_;
};

} // namespace app
} // namespace ocs
