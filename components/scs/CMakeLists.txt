idf_component_register(
    SRCS
    "project_pipeline.cpp"
    "control_pipeline.cpp"

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
