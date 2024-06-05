#pragma once

#include "noncopyable.h"

class cJSONFormatter : public NonCopyable<> {
public:
    //! Initialize.
    explicit cJSONFormatter(cJSON* json);

    //! Append 16-bit number stored in @p value.
    //!
    //! @remarks
    //!  - @p json should be an array.
    //!
    //! @return
    //!  True if data was added to @p json properly;
    //!  False if some error occurred.
    bool append_uint16(uint16_t value, cJSON* json);

    //! Add constant string @p value with constant @p key to json.
    bool add_string_ref_cs(const char* key, const char* value);

    //! Add @p value with constant @p key to json.
    bool add_string_cs(const char* key, const char* value);

    //! Add @p value with constant @p key to json.
    bool add_number_cs(const char* key, double value);

    //! Add boolean @p value with constant @p key to json.
    bool add_bool_cs(const char* key, bool value);

    //! Add True with constant @p key to json.
    bool add_true_cs(const char* key);

    //! Add False with constant @p key to json.
    bool add_false_cs(const char* key);

    //! Add Null with constant @p key to json.
    bool add_null_cs(const char* key);

private:
    cJSON* json_ { nullptr };
};
