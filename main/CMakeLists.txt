idf_component_register(
    SRCS
    "main.cpp"
    "adc_reader.cpp"
    "soil_moisture_monitor.cpp"
    "yl69_moisture_reader.cpp"
    "console_json_writer.cpp"
    "cjson_object_formatter.cpp"
    "cjson_array_formatter.cpp"
    "status_code_to_str.cpp"
    "gpio_config.cpp"
    "types.cpp"

    REQUIRES
    "freertos"
    "json"
    "esp_adc"
    "driver"

    INCLUDE_DIRS
    "."
)
