#include <cstdio>
#include <string>

#include "console_json_writer.h"
#include "types.h"

StatusCode ConsoleJSONWriter::write(const cJSONSharedBuilder::Ptr& json) {
    const auto str = std::string(cJSON_PrintUnformatted(json.get()));
    fprintf(stderr, "data=%s\n", str.c_str());

    return StatusCode::OK;
}
