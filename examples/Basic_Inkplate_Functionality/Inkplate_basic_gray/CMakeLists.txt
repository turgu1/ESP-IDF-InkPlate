cmake_minimum_required(VERSION 3.16.0)
set(IDF_TARGET esp32)

set(SDKCONFIG "${CMAKE_BINARY_DIR}/sdkconfig")

add_compile_definitions(
    EPUB_LOG_LEVEL=ESP_LOG_INFO

)

if(DEVICE MATCHES "^INKPLATE_6$")
    message("INKPLATE 6 defined")
    add_compile_definitions(
        INKPLATE_6=1 
        MCP23017=1)
elseif(DEVICE MATCHES  "^INKPLATE_6PLUS$")
    message("INKPLATE 6PLUS defined")
    add_compile_definitions(
        INKPLATE_6PLUS=1 
        MCP23017=1)
elseif(DEVICE MATCHES "^INKPLATE_6PLUS_V2$")
    message("INKPLATE 6PLUS V2 defined")
    add_compile_definitions(
        INKPLATE_6PLUS_V2=1 
        PCAL6416=1)
elseif(DEVICE MATCHES "^INKPLATE_10$")
    message("INKPLATE 10 defined")
    add_compile_definitions(
        INKPLATE_10=1 
        MCP23017=1)
elseif(DEVICE MATCHES "^INKPLATE_10_V2$")
    message("INKPLATE 10 V2 defined")
    add_compile_definitions(
        INKPLATE_10_V2=1 
        PCAL6416=1)
else()
    message(FATAL_ERROR "DEVICE NOT defined!!\nExample call: idf.py build -DDEVICE=INKPLATE_6\nDEVICE Options are:\n  INKPLATE_6\n  INKPLATE_6PLUS\n  INKPLATE_6PLUS_V2\n  INKPLATE_10\n  INKPLATE_10_V2")
endif()

list(APPEND EXTRA_COMPONENT_DIRS 
    "../../../../ESP-IDF-InkPlate"
    src 
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(Inkplate_basic_gray)
