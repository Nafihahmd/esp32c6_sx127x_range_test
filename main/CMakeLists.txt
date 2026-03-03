idf_component_register(SRCS "lora_range_main.c" "lora.c"
                    PRIV_REQUIRES driver  esp_driver_gpio esp_driver_spi
                    INCLUDE_DIRS ".")
