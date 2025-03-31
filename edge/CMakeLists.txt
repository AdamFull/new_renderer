cmake_minimum_required(VERSION 3.22.1)
project(edge_framework)

add_library(${PROJECT_NAME} STATIC)

if(ANDROID)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_ANDROID)
elseif(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_LINUX)
endif()

target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Edge.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Edge.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/EntryPoint.h"
)

add_subdirectory("util")
add_subdirectory("runtime")
add_subdirectory("fs")
add_subdirectory("gfx")

file(GLOB EDGE_FRAMEWORK_SOURCES FILES *.cpp *.h)

target_include_directories(${PROJECT_NAME} 
    SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    external_libraries
)