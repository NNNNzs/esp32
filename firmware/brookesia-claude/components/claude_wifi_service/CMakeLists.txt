idf_component_register(
    SRCS "src/claude_wifi_service.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi esp_netif esp_event claude_config_service
)
