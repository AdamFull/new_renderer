cmake_minimum_required(VERSION 3.18.0)
project(gfx_vulkan)

include(GenerateExportHeader)

set(ENABLE_VULKAN_VALIDATION_LAYERS ON CACHE BOOL "Enable validation layers for every application.")
set(ENABLE_VULKAN_VALIDATION_LAYERS_GPU_ASSISTED OFF CACHE BOOL "Enable GPU assisted validation layers for every application.")
set(ENABLE_VULKAN_VALIDATION_LAYERS_BEST_PRACTICES OFF CACHE BOOL "Enable best practices validation layers for every application.")
set(ENABLE_VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION ON CACHE BOOL "Enable synchronization validation layers for every application.")
set(ENABLE_VULKAN_DEBUG ON CACHE BOOL "Enable VK_EXT_debug_utils or VK_EXT_debug_marker if supported.")

find_package(volk CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)

file(GLOB_RECURSE VULKAN_RENDERER_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${VULKAN_RENDERER_SOURCES})

if(${ENABLE_VULKAN_DEBUG})
    if(${ENABLE_VULKAN_VALIDATION_LAYERS})
        target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_VALIDATION_LAYERS)

           # GPU assisted validation layers are not available on macOS.
        if(${ENABLE_VULKAN_VALIDATION_LAYERS_GPU_ASSISTED})
            if (APPLE)
                message(WARNING "GPU assisted validation layers are not currently available on macOS.")
            else()
                target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_VALIDATION_LAYERS_GPU_ASSISTED)
            endif()
        endif()
        
        if(${ENABLE_VULKAN_VALIDATION_LAYERS_BEST_PRACTICES})
            target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_VALIDATION_LAYERS_BEST_PRACTICES)
        endif()
        
        if(${ENABLE_VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION})
            target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION)
        endif()
    endif()

    target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_DEBUG)
endif()

if(${ENABLE_VULKAN_PORTABILITY})
    message(STATUS "Vulkan Portability Enumeration and Portability Subset extensions are enabled")
    target_compile_definitions(${PROJECT_NAME} PUBLIC VULKAN_ENABLE_PORTABILITY)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

add_library(vulkan_common INTERFACE)
target_link_libraries(vulkan_common
    INTERFACE
    Vulkan::Headers
    volk::volk 
    volk::volk_headers
    GPUOpen::VulkanMemoryAllocator
    GPUOpen::VulkanMemoryAllocatorHpp
)
target_compile_definitions(vulkan_common 
    INTERFACE
    VMA_STATIC_VULKAN_FUNCTIONS=0
    VMA_DYNAMIC_VULKAN_FUNCTIONS=1
    VMA_STATS_STRING_ENABLED=0 

    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

if(USE_NSIGHT_AFTERMATH)
    target_link_libraries(${PROJECT_NAME} PUBLIC nsight_aftermath_interface)
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    vulkan_common
	spirv-cross-core
	spirv-cross-glsl
)