idf_component_register(
    SRCS "app_gpio.c"
    INCLUDE_DIRS "include"
    REQUIRES chip esp_common
    PRIV_REQUIRES esp_driver_gpio
)
