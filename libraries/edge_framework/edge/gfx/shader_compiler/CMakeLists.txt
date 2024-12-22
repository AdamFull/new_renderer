cmake_minimum_required(VERSION 3.18.0)
project(gfx_shader_compiler)

find_package(glslang CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)

file(GLOB GFX_SHADER_COMPILER_SOURCES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${GFX_SHADER_COMPILER_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${PROJECT_NAME} 
	PRIVATE
	glslang::glslang
	glslang::SPIRV
    spirv-cross-core
	spirv-cross-glsl
	spirv-cross-msl
	spirv-cross-hlsl
)