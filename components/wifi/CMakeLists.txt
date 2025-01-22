idf_component_register(
    SRCS "src/wifi.c"
    INCLUDE_DIRS "include"
    REQUIRES queue_manager esp_wifi nvs_flash
)