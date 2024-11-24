cmake_minimum_required(VERSION 3.18.0)
project(shader_compiler)

find_package(directx-dxc CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)

file(GLOB_RECURSE COMPILER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE COMPILER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(${PROJECT_NAME} ${COMPILER_SOURCES} ${COMPILER_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TOOLS_BINARIES_PATH}/${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${TOOLS_BINARIES_PATH}/${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${TOOLS_BINARIES_PATH}/${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${TOOLS_BINARIES_PATH}/${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${TOOLS_BINARIES_PATH}/${PROJECT_NAME}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE utils
    PRIVATE Microsoft::DirectXShaderCompiler
	PRIVATE glslang::glslang
	PRIVATE glslang::SPIRV
	PRIVATE utility_lib
    PRIVATE spirv-cross-core
	PRIVATE spirv-cross-glsl
	PRIVATE spirv-cross-msl
	PRIVATE spirv-cross-hlsl
)