#include <cassert>

#include "cjson_array_formatter.h"
#include "cjson_builder.h"

cJSONArrayFormatter::cJSONArrayFormatter(cJSON* json)
    : json_(json) {
    assert(cJSON_IsArray(json_));
}

bool cJSONArrayFormatter::append_uint16(uint16_t value) {
    auto item = cJSONUniqueBuilder::make_json_number(value);
    if (!item) {
        return false;
    }

    if (!cJSON_AddItemToArray(json_, item.get())) {
        return false;
    }

    item.release();
    return true;
}
