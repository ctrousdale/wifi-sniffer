idf_component_register(
    SRCS "src/usb.cpp"
    INCLUDE_DIRS "include"
    REQUIRES queue_manager driver 
)