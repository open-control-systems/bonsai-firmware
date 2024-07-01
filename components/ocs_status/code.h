#pragma once

namespace ocs {
namespace status {

//! Status code.
enum class StatusCode {
    OK,     //!< Status indicating a success of an operation.
    NoData, //!< There is no enough data to perform an operation.
};

} // namespace status
} // namespace ocs
