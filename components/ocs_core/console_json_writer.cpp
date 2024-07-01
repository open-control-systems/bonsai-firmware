#include <cstdio>
#include <string>

#include "ocs_core/console_json_writer.h"

namespace ocs {
namespace core {

status::StatusCode ConsoleJSONWriter::write(const cJSONSharedBuilder::Ptr& json) {
    const auto str = std::string(cJSON_PrintUnformatted(json.get()));
    fprintf(stderr, "data=%s\n", str.c_str());

    return status::StatusCode::OK;
}

} // namespace core
} // namespace ocs
