cmake_minimum_required(VERSION 3.18.0)
project(engine_world)

find_package(meshoptimizer CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)

file(GLOB_RECURSE ENGINE_WORLD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE ENGINE_WORLD_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(ENGINE_WORLD_SOURCES
    ${ENGINE_WORLD_SOURCES}
    ${ENGINE_WORLD_HEADERS}
)

add_library(${PROJECT_NAME} STATIC ${ENGINE_WORLD_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/"
    PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/engine"
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE meshoptimizer::meshoptimizer
    PRIVATE EnTT::EnTT
)

target_precompile_headers(${PROJECT_NAME} PUBLIC world_pch.h)