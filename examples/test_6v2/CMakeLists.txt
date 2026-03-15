cmake_minimum_required(VERSION 3.16.0)

set(DEVICE "INKPLATE_6V2")

include("../SetupApp.txt")

add_compile_options("-O3")

list(APPEND EXTRA_COMPONENT_DIRS 
    ../../../ESP-IDF-InkPlate
    src 
)

project(test_6FLICK)

