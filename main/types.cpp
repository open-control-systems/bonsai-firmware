#include "types.h"

const char* soil_status_to_str(SoilStatus status) {
    switch (status) {
    case SoilStatus::Dry:
        return "dry";

    case SoilStatus::Wet:
        return "wet";

    default:
        break;
    }

    return "<none>";
}

SoilStatus parse_soil_status(unsigned status) {
    if (status >= static_cast<unsigned>(SoilStatus::Last)) {
        return SoilStatus::None;
    }

    return static_cast<SoilStatus>(status);
}

const char* soil_moisture_characteristic_to_str(SoilMoistureCharacteristic c) {
    switch (c) {
    case SoilMoistureCharacteristic::Raw:
        return "raw";

    case SoilMoistureCharacteristic::Voltage:
        return "voltage";

    case SoilMoistureCharacteristic::Status:
        return "status";

    default:
        break;
    }

    return "<none>";
}
