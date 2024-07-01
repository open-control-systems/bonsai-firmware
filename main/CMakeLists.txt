idf_component_register(
    SRCS
    "main.cpp"
    "adc_reader.cpp"
    "soil_moisture_monitor.cpp"
    "yl69_moisture_reader.cpp"
    "console_telemetry_writer.cpp"
    "gpio_config.cpp"
    "types.cpp"

    REQUIRES
    "freertos"
    "json"
    "esp_adc"
    "driver"
    "ocs_core"
    "ocs_status"

    INCLUDE_DIRS
    "."
)
