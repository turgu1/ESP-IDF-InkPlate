FILE(GLOB_RECURSE inkplate_srcs src/**/*.cpp)
FILE(GLOB inkplate_include_dirs include/*)
LIST(FILTER inkplate_include_dirs EXCLUDE REGEX ".*/README$")

if(CONFIG_INKPLATE_VARIANT_10)
    set(INKPLATE_VARIANT "INKPLATE_10")
elseif(CONFIG_INKPLATE_VARIANT_6PLUS)
    set(INKPLATE_VARIANT "INKPLATE_6PLUS")
    elseif(CONFIG_INKPLATE_VARIANT_6PLUSV2)
    set(INKPLATE_VARIANT "INKPLATE_6PLUSV2")
elseif(CONFIG_INKPLATE_VARIANT_6)
    set(INKPLATE_VARIANT "INKPLATE_6")
endif()

idf_component_register(
    SRCS ${inkplate_srcs}
    INCLUDE_DIRS ${inkplate_include_dirs}
    REQUIRES fatfs nvs_flash esp_http_client
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC ${INKPLATE_VARIANT})
