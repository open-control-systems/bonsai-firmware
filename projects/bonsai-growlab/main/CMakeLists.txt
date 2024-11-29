idf_component_register(
    SRCS
    "main.cpp"
    "project_pipeline.cpp"
    "control_pipeline.cpp"
    "ds18b20_pipeline.cpp"
    "soil_pipeline.cpp"
    "sht41_pipeline.cpp"

    REQUIRES
    "freertos"
    "ocs_core"
    "ocs_status"
    "ocs_net"
    "ocs_storage"
    "ocs_system"
    "ocs_diagnostic"
    "ocs_scheduler"
    "ocs_io"
    "ocs_sensor"
    "ocs_pipeline"

    INCLUDE_DIRS
    ".."
)

set(WEB_GUI_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../web-gui")
set(WEB_GUI_BUILD_DIR "${WEB_GUI_SRC_DIR}/dist")

if(EXISTS ${WEB_GUI_BUILD_DIR})
    spiffs_create_partition_image(web_gui ${WEB_GUI_BUILD_DIR} FLASH_IN_PROJECT)
else()
    message(FATAL_ERROR "${WEB_GUI_BUILD_DIR} doesn't exist. Please run 'npm run build' in ${WEB_GUI_SRC_DIR}")
endif()
