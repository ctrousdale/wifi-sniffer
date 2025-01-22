idf_component_register(
    SRCS "src/usb.c"
    INCLUDE_DIRS "include"
    # REQUIRES esp_usb
    REQUIRES queue_manager driver 
)