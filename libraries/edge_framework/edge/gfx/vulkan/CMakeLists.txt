include(GenerateExportHeader)

set(ENABLE_VULKAN_VALIDATION_LAYERS ON CACHE BOOL "Enable validation layers for every application.")
set(ENABLE_VULKAN_VALIDATION_LAYERS_GPU_ASSISTED OFF CACHE BOOL "Enable GPU assisted validation layers for every application.")
set(ENABLE_VULKAN_VALIDATION_LAYERS_BEST_PRACTICES OFF CACHE BOOL "Enable best practices validation layers for every application.")
set(ENABLE_VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION ON CACHE BOOL "Enable synchronization validation layers for every application.")
set(ENABLE_VULKAN_DEBUG ON CACHE BOOL "Enable VK_EXT_debug_utils or VK_EXT_debug_marker if supported.")

file(GLOB EDGE_GFX_VULKAN_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)
target_sources(${PROJECT_NAME} PRIVATE
    "${EDGE_GFX_VULKAN_SOURCES}"
)

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

target_compile_definitions(${PROJECT_NAME} 
    PRIVATE
    VMA_STATIC_VULKAN_FUNCTIONS=0
    VMA_DYNAMIC_VULKAN_FUNCTIONS=1
    VMA_STATS_STRING_ENABLED=0 

    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

if(USE_NSIGHT_AFTERMATH)
    target_link_libraries(${PROJECT_NAME} PRIVATE nsight_aftermath_interface)
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Vulkan::Headers
    volk::volk
    GPUOpen::VulkanMemoryAllocator
)