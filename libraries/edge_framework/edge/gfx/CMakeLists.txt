cmake_minimum_required(VERSION 3.18.0)
project(edge_gfx)

#glslang
set(BUILD_EXTERNAL OFF CACHE BOOL "Build external dependencies in /External" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Builds glslangValidator and spirv-remap" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "Enables testing" FORCE)

#ktx
set(KTX_FEATURE_TESTS OFF CACHE BOOL "Create unit tests" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "Create KTX tools" FORCE)
set(KTX_FEATURE_VULKAN OFF CACHE BOOL "Enable Vulkan texture upload" FORCE)
set(KTX_FEATURE_GL_UPLOAD OFF CACHE BOOL "Enable OpenGL texture upload" FORCE)

#spirv-cross
#set(SPIRV_CROSS_SHARED OFF CACHE BOOL "Build the C API as a single shared library." FORCE)
set(SPIRV_CROSS_STATIC ON CACHE BOOL "Build the C and C++ API as static libraries." FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "Build the CLI binary. Requires SPIRV_CROSS_STATIC." FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Enable SPIRV-Cross tests." FORCE)
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "Enable util module support." FORCE)

find_package(Ktx CONFIG REQUIRED)

add_subdirectory(shader_compiler)

file(GLOB GFX_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${GFX_SOURCES})

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

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
    KTX::ktx
    PUBLIC
    gfx_shader_compiler
)

add_subdirectory(shaders)