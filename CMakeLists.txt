FILE(GLOB_RECURSE inkplate_srcs src/**/*.*)
FILE(GLOB inkplate_include_dirs src/*)
LIST(FILTER inkplate_include_dirs EXCLUDE REGEX ".*/CMakeLists.txt$")


set(components
    fatfs
    nvs_flash
    esp_http_client
    esp_driver_i2c
    esp_driver_i2s
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

