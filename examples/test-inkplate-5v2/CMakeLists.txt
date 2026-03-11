# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)


add_compile_definitions(
    INKPLATE_5V2=1 
    PCAL6416=1
)

list(APPEND EXTRA_COMPONENT_DIRS
    "../../../ESP-IDF-InkPlate"
    src
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(test-inkplate-5v2)