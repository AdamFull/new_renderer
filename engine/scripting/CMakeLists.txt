cmake_minimum_required(VERSION 3.18.0)
project(script_engine)

add_library(mono_static STATIC IMPORTED)
set_property(TARGET mono_static PROPERTY IMPORTED_LOCATION "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/Mono/lib/libmono-static-sgen.lib")

file(GLOB ENGINE_SCRIPT_ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ENGINE_SCRIPT_ENGINE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(${PROJECT_NAME} STATIC ${ENGINE_SCRIPT_ENGINE_SOURCES} ${ENGINE_SCRIPT_ENGINE_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/"
    PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/engine"
    PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/Mono/include"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE mono_static
)