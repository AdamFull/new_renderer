cmake_minimum_required(VERSION 3.18.0)
project(sfx_openal)

find_package(OpenAL CONFIG REQUIRED)

file(GLOB_RECURSE SFX_OPENAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} STATIC ${SFX_OPENAL_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE OpenAL::OpenAL
)