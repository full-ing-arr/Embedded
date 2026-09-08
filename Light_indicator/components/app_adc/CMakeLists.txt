idf_component_register(
    SRCS "app_adc.c"
    INCLUDE_DIRS "include"
    REQUIRES chip esp_adc
    PRIV_REQUIRES esp_hal_ana_conv
)
