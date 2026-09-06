idf_component_register(
    SRCS "app_gpio.c" "app_gpio_interrupt.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_driver_gpio freertos
)
