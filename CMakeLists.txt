FILE(GLOB_RECURSE inkplate_srcs src/**/*.cpp)
FILE(GLOB inkplate_include_dirs include/*)
LIST(FILTER inkplate_include_dirs EXCLUDE REGEX ".*/README$")

idf_component_register(
    SRCS ${inkplate_srcs}
    INCLUDE_DIRS ${inkplate_include_dirs}
)
