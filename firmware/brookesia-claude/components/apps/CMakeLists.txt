set(APPS_DIR ./)
file(GLOB_RECURSE APPS_C_SRCS ${APPS_DIR}/*.c)
file(GLOB_RECURSE APPS_CPP_SRCS ${APPS_DIR}/*.cpp)

idf_component_register(
    SRCS ${APPS_C_SRCS} ${APPS_CPP_SRCS}
    INCLUDE_DIRS ${APPS_DIR}
    REQUIRES lvgl__lvgl esp_event esp_wifi nvs_flash bsp_extra esp_driver_jpeg esp_mm esp-brookesia esp32_s3_touch_lcd_1_83 espressif__esp_lcd_touch_cst816s claude_status_service claude_config_web claude_wifi_service)

target_compile_options(
    ${COMPONENT_LIB}
    PRIVATE
        -Wno-format
        -DLV_LVGL_H_INCLUDE_SIMPLE)

target_compile_options(
    ${COMPONENT_LIB}
    PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-Wno-incompatible-pointer-types>
        $<$<COMPILE_LANGUAGE:C>:-Wno-int-conversion>)
