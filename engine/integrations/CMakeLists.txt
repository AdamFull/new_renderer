cmake_minimum_required(VERSION 3.18.0)
project(integrations)

add_subdirectory(imgui)

file(GLOB INTEGRATIONS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB INTEGRATIONS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(${PROJECT_NAME} STATIC ${INTEGRATIONS_SOURCES} ${INTEGRATIONS_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/engine"
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC imgui_integration
)