idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        apps
        bsp_extra
        claude_wifi_service
        claude_mqtt_service
        claude_config_web
        esp_driver_gpio
        esp_driver_uart
        esp_event
        esp_netif
        espressif__esp-brookesia
        espressif__esp_lvgl_port
        nvs_flash
        waveshare__esp32_s3_touch_lcd_1_83)

target_compile_options(${COMPONENT_LIB} PUBLIC -Wno-missing-field-initializers)

# If want to use custom memory allocation functions, refer to the following macros:
if(CONFIG_ESP_BROOKESIA_MEMORY_USE_CUSTOM)
    # Get the exact component name
    idf_build_get_property(build_components BUILD_COMPONENTS)
    foreach(COMPONENT ${build_components})
        if(COMPONENT MATCHES "esp-brookesia" OR COMPONENT MATCHES "espressif__esp-brookesia")
            set(TARGET_COMPONENT ${COMPONENT})
            break()
        endif()
    endforeach()

    # Get the component library
    if(TARGET_COMPONENT STREQUAL "")
        message(FATAL_ERROR "Component 'esp-brookesia' not found.")
    else()
        idf_component_get_property(ESP_BROOKESIA_LIB ${TARGET_COMPONENT} COMPONENT_LIB)
    endif()

    # Add the custom memory allocation functions
    target_compile_options(
        ${ESP_BROOKESIA_LIB}
        PUBLIC
            "-DESP_BROOKESIA_MEMORY_CUSTOM_MALLOC(x)=heap_caps_aligned_alloc(1, x, MALLOC_CAP_SPIRAM)"  "-DESP_BROOKESIA_MEMORY_CUSTOM_FREE=free"
    )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

spiffs_create_partition_image(
    storage
    ../spiffs
    FLASH_IN_PROJECT
    SPIFFS_IMAGE_FLAGS="-b 4096 -p 256 -s 0x100000 -q"
)
