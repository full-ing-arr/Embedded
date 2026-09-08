idf_component_register(
    INCLUDE_DIRS 
        "include"
        "profiles/${IDF_TARGET}"
    REQUIRES esp_hal_ana_conv
)
