cmake_minimum_required(VERSION 3.18.0)
project(gfx_vulkan)

include(GenerateExportHeader)

set(ENABLE_VULKAN_VALIDATION_LAYERS ON CACHE BOOL "Enable validation layers for every application.")
set(ENABLE_VULKAN_VALIDATION_LAYERS_GPU_ASSISTED OFF CACHE BOOL "Enable GPU assisted validation layers for every application.")
set(ENABLE_VULKAN_VALIDATION_LAYERS_BEST_PRACTICES OFF CACHE BOOL "Enable best practices validation layers for every application.")
set(ENABLE_VULKAN_VALIDATION_LAYERS_SYNCHRONIZATION ON CACHE BOOL "Enable synchronization validation layers for every application.")
set(ENABLE_VULKAN_DEBUG ON CACHE BOOL "Enable VK_EXT_debug_utils or VK_EXT_debug_marker if supported.")

find_package(volk CONFIG REQUIRED)
find_package(VulkanHeaders CONFIG)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)

add_subdirectory("${MAIN_PROJECT_DIRECTORY_PATH}/libraries/vma" vma)

set(VULKAN_RENDERER_DEFINITIONS 
	"VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"
	"VK_NO_PROTOTYPES"
)

# Add definitions for win32
if(WIN32)
	set(VULKAN_RENDERER_DEFINITIONS ${VULKAN_RENDERER_DEFINITIONS} "VK_USE_PLATFORM_WIN32_KHR")
endif()

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

target_include_directories(${PROJECT_NAME} 
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../"
	PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/vma/include"
	PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/Vulkan-Headers/include"
	PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/tracy/public"
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE utility_lib
    PRIVATE volk::volk 
    PRIVATE volk::volk_headers
	PRIVATE Vulkan::Headers
	PRIVATE VulkanMemoryAllocator
	PRIVATE spirv-cross-core
	PRIVATE spirv-cross-glsl
)

target_compile_definitions(${PROJECT_NAME} 
	PUBLIC ${VULKAN_RENDERER_DEFINITIONS}
)