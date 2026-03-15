
FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

set(CMAKE_CXX_STANDARD 17)

set(components
    ESP-IDF-InkPlate
    spi_flash
)

idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "." REQUIRES ${components})
