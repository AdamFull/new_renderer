cmake_minimum_required(VERSION 3.18.0)
project(edge_filesystem_new)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
	"fs_virtual_filesystem.cpp" 
)

add_subdirectory("native")
add_subdirectory("zip")

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)