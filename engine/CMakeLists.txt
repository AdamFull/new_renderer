cmake_minimum_required(VERSION 3.18.0)
project(application)
 
find_package(EnTT CONFIG REQUIRED)

file(GLOB ENGINE_APP_SOURCES FILES *.cpp *.h)
file(GLOB_RECURSE ENGINE_RENDERING_SOURCES FILES rendering/*cpp)
file(GLOB_RECURSE ENGINE_WORLD_SOURCES FILES world/*cpp)
file(GLOB_RECURSE ENGINE_UI_SOURCES FILES ui/*cpp)

add_library(${PROJECT_NAME} STATIC 
    ${ENGINE_APP_SOURCES}
    ${ENGINE_RENDERING_SOURCES}
    ${ENGINE_WORLD_SOURCES}
    ${ENGINE_UI_SOURCES}
)

if(MSVC)
	# mask out the min/max macros from minwindef.h
    add_definitions(-DNOMINMAX)
	target_compile_options(${PROJECT_NAME} PUBLIC /MP)
    add_compile_options(/wd4996)
else()
	target_compile_options(${PROJECT_NAME} PUBLIC -fexceptions)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_library(engine_common INTERFACE)
target_link_libraries(engine_common
    INTERFACE
    glm 
    EnTT::EnTT 
    imdbg 
    imgui
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    tinygltf
    edge_framework
    engine_common 
)