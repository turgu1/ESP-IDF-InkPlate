
FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

set(components
    ESP-IDF-InkPlate
)

idf_component_register(SRCS ${app_sources} INCLUDE_DIRS "." REQUIRES ${components})
