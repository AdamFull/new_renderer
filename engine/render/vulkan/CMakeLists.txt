cmake_minimum_required(VERSION 3.18.0)
project(vulkan_renderer)

include(GenerateExportHeader)

add_subdirectory("${MAIN_PROJECT_DIRECTORY_PATH}/libraries/Vulkan-Headers" Vulkan-Headers)
add_subdirectory("${MAIN_PROJECT_DIRECTORY_PATH}/libraries/vma" vma)
add_subdirectory("${MAIN_PROJECT_DIRECTORY_PATH}/libraries/SPIRV-Cross" SPIRV-Cross)

set(VULKAN_RENDERER_DEFINITIONS 
	"VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"
	"VK_NO_PROTOTYPES"
)

# Add definitions for win32
if(WIN32)
	set(VULKAN_RENDERER_DEFINITIONS ${VULKAN_RENDERER_DEFINITIONS} "VK_USE_PLATFORM_WIN32_KHR")
endif()

file(GLOB_RECURSE VULKAN_RENDERER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE VULKAN_RENDERER_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(${PROJECT_NAME} STATIC ${VULKAN_RENDERER_SOURCES} ${VULKAN_RENDERER_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
	PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/engine"
	PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/utllib"
	PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/vma/include"
	PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/Vulkan-Headers/include"
	PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/SPIRV-Cross"
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC utility_lib
	PUBLIC Vulkan-Headers
	PUBLIC VulkanMemoryAllocator
	PUBLIC spirv-cross-core
	PUBLIC spirv-cross-glsl
)

target_compile_definitions(${PROJECT_NAME} 
	PUBLIC ${VULKAN_RENDERER_DEFINITIONS}
)