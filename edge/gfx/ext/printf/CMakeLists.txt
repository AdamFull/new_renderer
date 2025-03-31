cmake_minimum_required(VERSION 3.18.0)
project(edge_gfx_ext_printf)

file(GLOB GFX_EXT_PRINTF_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${GFX_EXT_PRINTF_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)