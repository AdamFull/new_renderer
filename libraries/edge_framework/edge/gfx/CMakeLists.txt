cmake_minimum_required(VERSION 3.18.0)
project(edge_gfx)

#spirv-cross
#set(SPIRV_CROSS_SHARED OFF CACHE BOOL "Build the C API as a single shared library." FORCE)
set(SPIRV_CROSS_STATIC ON CACHE BOOL "Build the C and C++ API as static libraries." FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "Build the CLI binary. Requires SPIRV_CROSS_STATIC." FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Enable SPIRV-Cross tests." FORCE)
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "Enable util module support." FORCE)

set(USE_NSIGHT_AFTERMATH ON CACHE BOOL "Enable nsight aftermath support." FORCE)

#TODO: add extensions as plugins
add_subdirectory("ext/resource_loader")
add_subdirectory("ext/shader_effect_system")

file(GLOB GFX_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${GFX_SOURCES})

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
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    edge_gfx_ext_resource_loader
    edge_gfx_ext_shader_effect_system
)