cmake_minimum_required(VERSION 3.18.0)
project(edge_gfx_ext_shader_effect_system)

#glslang
set(BUILD_EXTERNAL OFF CACHE BOOL "Build external dependencies in /External" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Builds glslangValidator and spirv-remap" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "Enables testing" FORCE)

find_package(glslang CONFIG REQUIRED)

file(GLOB GFX_SHADER_EFFECT_SYSTEM_SOURCES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${GFX_SHADER_EFFECT_SYSTEM_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${PROJECT_NAME} 
	PRIVATE
	glslang::glslang
	glslang::SPIRV
)