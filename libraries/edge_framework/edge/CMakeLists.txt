cmake_minimum_required(VERSION 3.22.1)
project(edge_framework)

add_subdirectory("util")
add_subdirectory("runtime")
add_subdirectory("filesystem")
add_subdirectory("gfx")

file(GLOB EDGE_FRAMEWORK_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${EDGE_FRAMEWORK_SOURCES})

if(ANDROID)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_ANDROID)
elseif(WIN32)
    add_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_MACOS)
elseif(UNIX)
    add_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_LINUX)
endif()

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE edge_utils
    PRIVATE edge_runtime
    PRIVATE edge_filesystem
    PRIVATE edge_gfx
)