idf_component_register(
    SRCS "src/claude_mqtt_service.cpp"
    INCLUDE_DIRS "include"
    REQUIRES mqtt esp_event claude_status_service claude_config_service
)
