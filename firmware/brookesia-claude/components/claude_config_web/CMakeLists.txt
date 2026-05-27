idf_component_register(
    SRCS "src/claude_config_web.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_http_server claude_config_service
)
