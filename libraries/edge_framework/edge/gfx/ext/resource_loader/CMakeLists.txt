cmake_minimum_required(VERSION 3.18.0)
project(edge_gfx_ext_resource_loader)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/ShaderEmbedding.cmake)

#ktx
set(KTX_FEATURE_TESTS OFF CACHE BOOL "Create unit tests" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "Create KTX tools" FORCE)
set(KTX_FEATURE_VULKAN OFF CACHE BOOL "Enable Vulkan texture upload" FORCE)
set(KTX_FEATURE_GL_UPLOAD OFF CACHE BOOL "Enable OpenGL texture upload" FORCE)

find_package(Ktx CONFIG REQUIRED)

file(GLOB GFX_EXT_RESOURCE_LOADER_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${GFX_EXT_RESOURCE_LOADER_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../external/stb"
)

add_shader_compile_glsl("${CMAKE_CURRENT_SOURCE_DIR}/shaders/mipgen.comp" GLSLANG_FLAGS ${GLSL_FLAGS} EMBED_TARGET MipGeneratorShader)
add_shader_compile_glsl("${CMAKE_CURRENT_SOURCE_DIR}/shaders/mipgen.comp" GLSLANG_FLAGS ${GLSL_FLAGS} DEFINES MIP_2 EMBED_TARGET MipGeneratorShaderMip2)
add_shader_compile_glsl("${CMAKE_CURRENT_SOURCE_DIR}/shaders/mipgen.comp" GLSLANG_FLAGS ${GLSL_FLAGS} DEFINES MIP_2 MIP_3 EMBED_TARGET MipGeneratorShaderMip3)
add_shader_compile_glsl("${CMAKE_CURRENT_SOURCE_DIR}/shaders/mipgen.comp" GLSLANG_FLAGS ${GLSL_FLAGS} DEFINES MIP_2 MIP_3 MIP_4 EMBED_TARGET MipGeneratorShaderMip4)

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
    KTX::ktx
    MipGeneratorShader
    MipGeneratorShaderMip2
    MipGeneratorShaderMip3
    MipGeneratorShaderMip4
)