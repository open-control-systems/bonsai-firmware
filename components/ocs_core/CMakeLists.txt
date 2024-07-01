idf_component_register(
    SRCS
    "console_json_writer.cpp"
    "cjson_object_formatter.cpp"
    "cjson_array_formatter.cpp"

    REQUIRES
    "json"

    INCLUDE_DIRS
    ".."
)
