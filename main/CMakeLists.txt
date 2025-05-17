idf_component_register(
    SRCS "main.cpp"
    REQUIRES queue_manager wifi usb
    INCLUDE_DIRS ""
)

# Removed module flag as modules are no longer used
