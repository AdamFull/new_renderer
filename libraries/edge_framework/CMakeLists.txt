cmake_minimum_required(VERSION 3.22.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(tools/shader_compiler)

add_subdirectory(edge)
add_subdirectory(examples/HelloTriangle)