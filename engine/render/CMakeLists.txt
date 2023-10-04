cmake_minimum_required(VERSION 3.18.0)
project(render_interface)

add_subdirectory(vulkan)

file(GLOB ENGINE_RENDER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ENGINE_RENDER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(${PROJECT_NAME} STATIC ${ENGINE_RENDER_SOURCES} ${ENGINE_RENDER_HEADERS})

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC vulkan_renderer
)