cmake_minimum_required(VERSION 3.18.0)
project(edge_gfx)

include(GlslShaderCompiler)

#ktx
set(KTX_FEATURE_TESTS OFF CACHE BOOL "Create unit tests" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "Create KTX tools" FORCE)
set(KTX_FEATURE_VULKAN OFF CACHE BOOL "Enable Vulkan texture upload" FORCE)
set(KTX_FEATURE_GL_UPLOAD OFF CACHE BOOL "Enable OpenGL texture upload" FORCE)

find_package(Ktx CONFIG REQUIRED)

#spirv-cross
#set(SPIRV_CROSS_SHARED OFF CACHE BOOL "Build the C API as a single shared library." FORCE)
set(SPIRV_CROSS_STATIC ON CACHE BOOL "Build the C and C++ API as static libraries." FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "Build the CLI binary. Requires SPIRV_CROSS_STATIC." FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Enable SPIRV-Cross tests." FORCE)
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "Enable util module support." FORCE)

set(USE_NSIGHT_AFTERMATH OFF CACHE BOOL "Enable nsight aftermath support." FORCE)

#TODO: add extensions as plugins
add_subdirectory("ext/imgui")
add_subdirectory("ext/printf")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    "gfx_context.cpp"
    "gfx_enum.cpp"
    "gfx_format_vk.cpp"
    "gfx_frame_graph.cpp"
    "gfx_nsight_aftermath.cpp"
    "gfx_resource_loader.cpp"
    "gfx_string.cpp"
    "gfx_structures.cpp"
)

add_subdirectory("technique_system")

if(USE_NSIGHT_AFTERMATH)
find_package(NsightAftermath REQUIRED)
add_library(nsight_aftermath_interface INTERFACE)
target_compile_definitions(nsight_aftermath_interface INTERFACE USE_NSIGHT_AFTERMATH=1)
target_link_libraries(nsight_aftermath_interface INTERFACE NsightAftermath::NsightAftermath)

target_link_libraries(${PROJECT_NAME} PUBLIC nsight_aftermath_interface)
endif()

if(ANDROID)
    add_subdirectory(vulkan)
    target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
    gfx_vulkan)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VULKAN_SUPPORTED=1)
elseif(WIN32)
    add_subdirectory(directx12)
    add_subdirectory(vulkan)

    target_link_libraries(${PROJECT_NAME} 
        PUBLIC
        gfx_d3d12
        gfx_vulkan
    )

    target_compile_definitions(${PROJECT_NAME} 
        PRIVATE D3D12_SUPPORTED=1
        PRIVATE VULKAN_SUPPORTED=1
    )
elseif(APPLE)
    add_subdirectory(vulkan)

    target_link_libraries(${PROJECT_NAME} 
        PUBLIC
        gfx_vulkan
    )

    target_compile_definitions(${PROJECT_NAME} 
        PRIVATE VULKAN_SUPPORTED=1
    )
elseif(UNIX)
    add_subdirectory(vulkan)

    target_link_libraries(${PROJECT_NAME} 
        PUBLIC
        gfx_vulkan
    )

    target_compile_definitions(${PROJECT_NAME} 
        PRIVATE VULKAN_SUPPORTED=1
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}" 
    "${CMAKE_CURRENT_SOURCE_DIR}/../../external/stb"
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
    PUBLIC
    edge_gfx_ext_imgui
    edge_gfx_ext_printf
)