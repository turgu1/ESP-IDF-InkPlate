cmake_minimum_required(VERSION 3.16.0)

set(IDF_TARGET esp32)

include("../../../SetupApp.txt")

list(APPEND EXTRA_COMPONENT_DIRS 
    "../../../../ESP-IDF-InkPlate"
    src 
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(Inkplate_Maze_Generator)
