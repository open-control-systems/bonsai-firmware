idf_component_register(
    SRCS
    "wifi_network.cpp"

    REQUIRES
    "esp_netif"
    "esp_wifi"
    "ocs_core"

    INCLUDE_DIRS
    ".."
)
