cmake_minimum_required(VERSION 3.19.1)
project(imgui)

find_package(Freetype REQUIRED)

file(GLOB_RECURSE IMGUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCES})

target_link_libraries(${PROJECT_NAME} 
    PUBLIC Freetype::Freetype
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLFW_INCLUDE_DIR} 
)