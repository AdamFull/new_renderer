cmake_minimum_required(VERSION 3.18.0)
project(edge_gfx_ext_imgui)

include(GlslShaderCompiler)

file(GLOB GFX_EXT_IMGUI_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${GFX_EXT_IMGUI_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

add_shader_compile_glsl("${CMAKE_CURRENT_SOURCE_DIR}/shaders/imgui.vert" GLSLANG_FLAGS ${GLSL_FLAGS} EMBED_TARGET ImguiVertex)
add_shader_compile_glsl("${CMAKE_CURRENT_SOURCE_DIR}/shaders/imgui.frag" GLSLANG_FLAGS ${GLSL_FLAGS} EMBED_TARGET ImguiFragment)

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
    ImguiVertex
    ImguiFragment
)