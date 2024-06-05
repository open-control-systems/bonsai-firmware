#include "status_code_to_str.h"

const char* status_code_to_str(StatusCode status) {
    switch (status) {
    case StatusCode::OK:
        return "OK";
    case StatusCode::NoData:
        return "NoData";

    default:
        break;
    }

    return "<none>";
}
