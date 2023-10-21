cmake_minimum_required(VERSION 3.18.0)
project(shader_compiler)

add_subdirectory("${MAIN_PROJECT_DIRECTORY_PATH}/libraries/glslang" glslang)

file(GLOB_RECURSE COMPILER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE COMPILER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(${PROJECT_NAME} ${COMPILER_SOURCES} ${COMPILER_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${PROJECT_NAME} 
	PRIVATE glslang
    PRIVATE SPIRV
	PRIVATE utility_lib
)
target_include_directories(${PROJECT_NAME} 
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
	PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/glslang"
	PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/utllib"
)