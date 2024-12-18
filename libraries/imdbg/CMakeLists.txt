cmake_minimum_required(VERSION 3.18.0)
project(imdbg)

find_package(Freetype REQUIRED)

file(GLOB IMDBG_SOURCES "*.cpp")

add_library(${PROJECT_NAME} STATIC ${IMDBG_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE Freetype::Freetype
)