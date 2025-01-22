idf_component_register(
    SRCS 
        "src/queue_manager.c"
    INCLUDE_DIRS 
        "include"
    REQUIRES 
        freertos
)