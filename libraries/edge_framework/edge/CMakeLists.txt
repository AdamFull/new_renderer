cmake_minimum_required(VERSION 3.22.1)
project(edge_framework)

if(ANDROID)
    add_compile_definitions(PLATFORM_ANDROID)
elseif(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(PLATFORM_MACOS)
elseif(UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

add_library(${PROJECT_NAME} STATIC)
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
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    external_libraries
)