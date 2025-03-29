include(GlslShaderCompiler)

option(USE_NSIGHT_AFTERMATH OFF)

#TODO: add extensions as plugins
#add_subdirectory("ext/imgui")
#add_subdirectory("ext/printf")

file(GLOB EDGE_GFX_RESOURCE_LOADER_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/resource_loader/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/resource_loader/*.h")

file(GLOB EDGE_GFX_TECHNIQUE_SYSTEM_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/technique_system/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/technique_system/*.h")

file(GLOB EDGE_GFX_BASE_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

target_sources(${PROJECT_NAME} PRIVATE
    # resource loader
    "${EDGE_GFX_RESOURCE_LOADER_SOURCES}"

    # shader effect technique system
    "${EDGE_GFX_TECHNIQUE_SYSTEM_SOURCES}"

    # base
    "${EDGE_GFX_BASE_SOURCES}"
)

if(USE_NSIGHT_AFTERMATH)
find_package(NsightAftermath REQUIRED)
add_library(nsight_aftermath_interface INTERFACE)
target_compile_definitions(nsight_aftermath_interface INTERFACE USE_NSIGHT_AFTERMATH=1)
target_link_libraries(nsight_aftermath_interface INTERFACE NsightAftermath::NsightAftermath)

target_link_libraries(${PROJECT_NAME} PUBLIC nsight_aftermath_interface)
endif()

if(ANDROID)
    add_subdirectory(vulkan)
    target_compile_definitions(${PROJECT_NAME} PRIVATE VULKAN_SUPPORTED=1)
elseif(WIN32)
    add_subdirectory(directx12)
    add_subdirectory(vulkan)

    target_compile_definitions(${PROJECT_NAME} 
        PRIVATE 
        D3D12_SUPPORTED=1
        VULKAN_SUPPORTED=1
    )
elseif(APPLE)
    add_subdirectory(vulkan)

    target_compile_definitions(${PROJECT_NAME} 
        PRIVATE VULKAN_SUPPORTED=1
    )
elseif(UNIX)
    add_subdirectory(vulkan)

    target_compile_definitions(${PROJECT_NAME} 
        PRIVATE VULKAN_SUPPORTED=1
    )
endif()

add_shader_compile_glsl("${CMAKE_CURRENT_SOURCE_DIR}/shaders/mipgen.comp" GLSLANG_FLAGS ${GLSL_FLAGS} EMBED_TARGET MipGeneratorShader)
add_shader_compile_glsl("${CMAKE_CURRENT_SOURCE_DIR}/shaders/mipgen.comp" GLSLANG_FLAGS ${GLSL_FLAGS} DEFINES MIP_2 EMBED_TARGET MipGeneratorShaderMip2)
add_shader_compile_glsl("${CMAKE_CURRENT_SOURCE_DIR}/shaders/mipgen.comp" GLSLANG_FLAGS ${GLSL_FLAGS} DEFINES MIP_2 MIP_3 EMBED_TARGET MipGeneratorShaderMip3)
add_shader_compile_glsl("${CMAKE_CURRENT_SOURCE_DIR}/shaders/mipgen.comp" GLSLANG_FLAGS ${GLSL_FLAGS} DEFINES MIP_2 MIP_3 MIP_4 EMBED_TARGET MipGeneratorShaderMip4)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    MipGeneratorShader
    MipGeneratorShaderMip2
    MipGeneratorShaderMip3
    MipGeneratorShaderMip4
)