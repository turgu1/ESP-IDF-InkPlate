FILE(GLOB_RECURSE inkplate_srcs src/**/*.cpp)
FILE(GLOB inkplate_include_dirs include/*)
LIST(FILTER inkplate_include_dirs EXCLUDE REGEX ".*/README$")

set(components
    fatfs
    nvs_flash
    esp_http_client
    esp_driver_i2c
    esp_adc
    esp_timer
    freertos
    driver
    esp_wifi
    mbedtls
)

idf_component_register(
    SRCS ${inkplate_srcs}
    INCLUDE_DIRS ${inkplate_include_dirs}
    REQUIRES ${components}
)

