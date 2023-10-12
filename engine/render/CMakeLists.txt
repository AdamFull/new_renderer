cmake_minimum_required(VERSION 3.18.0)
project(render_interface)

add_subdirectory(vulkan)
add_subdirectory("${MAIN_PROJECT_DIRECTORY_PATH}/libraries/KTX-Software" KTX-Software)

file(GLOB ENGINE_RENDER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ENGINE_RENDER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(${PROJECT_NAME} STATIC ${ENGINE_RENDER_SOURCES} ${ENGINE_RENDER_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/engine"
    PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/KTX-Software/include"
    PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/KTX-Software/lib"
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC vulkan_renderer
    PRIVATE ktx
)

target_compile_definitions(${PROJECT_NAME} 
	PUBLIC "VK_NO_PROTOTYPES"
)