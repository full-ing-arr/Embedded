idf_component_register(
    SRCS "app_adc.c" "app_adc_interrupt.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_adc
    PRIV_REQUIRES freertos esp_hal_ana_conv
)