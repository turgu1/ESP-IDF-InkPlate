cmake_minimum_required(VERSION 3.5)

# change based on your device
# possible values: INKPLATE_6, INKPLATE_6PLUS, INKPLATE_10
add_compile_definitions(INKPLATE_10)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Hello_World)
