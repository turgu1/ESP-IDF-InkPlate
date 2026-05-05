cmake_minimum_required(VERSION 3.5)

set(IDF_TARGET esp32)

include("../../../SetupApp.txt")

list(APPEND EXTRA_COMPONENT_DIRS 
    "../../../../ESP-IDF-InkPlate"
    src 
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(Hello_World)
