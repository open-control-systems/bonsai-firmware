idf_component_register(
    SRCS
    "main.cpp"
    "adc_reader.cpp"
    "soil_moisture_monitor.cpp"
    "yl69_moisture_reader.cpp"
    "console_telemetry_writer.cpp"
    "gpio_config.cpp"
    "telemetry.cpp"
    "telemetry_formatter.cpp"
    "http_server.cpp"
    "fanout_telemetry_writer.cpp"
    "control_pipeline.cpp"

    REQUIRES
    "freertos"
    "json"
    "esp_adc"
    "driver"
    "esp_http_server"
    "ocs_core"
    "ocs_status"
    "ocs_net"
    "ocs_storage"

    INCLUDE_DIRS
    "."
)
