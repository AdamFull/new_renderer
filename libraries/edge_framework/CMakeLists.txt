cmake_minimum_required(VERSION 3.28.0)
project(edge_framework)

add_subdirectory("util")

file(GLOB EDGE_FRAMEWORK_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${EDGE_FRAMEWORK_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE edge_utils
)