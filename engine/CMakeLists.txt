cmake_minimum_required(VERSION 3.18.0)
project(application)

add_subdirectory(platform)
add_subdirectory(render)
add_subdirectory(filesystem)
add_subdirectory(scripting)

file(GLOB ENGINE_APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ENGINE_APP_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(${PROJECT_NAME} STATIC ${ENGINE_APP_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC platform_specific
	PUBLIC virtual_filesystem
	PRIVATE render_interface
	PRIVATE script_engine
)