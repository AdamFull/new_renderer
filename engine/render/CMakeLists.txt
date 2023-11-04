cmake_minimum_required(VERSION 3.18.0)
project(render_interface)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/render_api/vulkan")
add_subdirectory("${MAIN_PROJECT_DIRECTORY_PATH}/libraries/KTX-Software" KTX-Software)
add_subdirectory("${MAIN_PROJECT_DIRECTORY_PATH}/libraries/angelscript" ascript)

file(GLOB ENGINE_RENDER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ENGINE_RENDER_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

file(GLOB ENGINE_RENDER_RULES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/rendering_rules/*.cpp")
file(GLOB ENGINE_RENDER_RULES_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/rendering_rules/*.h")

add_library(${PROJECT_NAME} STATIC ${ENGINE_RENDER_SOURCES} ${ENGINE_RENDER_RULES_SOURCES} ${ENGINE_RENDER_HEADERS} ${ENGINE_RENDER_RULES_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/"
    PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/tools"
    PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/engine"
    PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/KTX-Software/include"
    PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/KTX-Software/lib"
    PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/angelscript/sdk/angelscript/include"
    PRIVATE "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/angelscript/sdk/add_on"
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC vulkan_renderer
    PRIVATE angelscript
    PRIVATE angelscript_addon
    PRIVATE ktx
)

target_compile_definitions(${PROJECT_NAME} 
	PUBLIC "VK_NO_PROTOTYPES"
)