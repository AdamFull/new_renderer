cmake_minimum_required(VERSION 3.18.0)
project(edge_runtime LANGUAGES CXX)

file(GLOB EDGE_RUNTIME_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${EDGE_RUNTIME_SOURCES})

if(WIN32)
    add_subdirectory(common)
    add_subdirectory(win64)

    target_link_libraries(${PROJECT_NAME} 
    PRIVATE edge_runtime_common
    PRIVATE edge_runtime_windows
    )
elseif(ANDROID)
    add_subdirectory(android)
    target_link_libraries(${PROJECT_NAME} PRIVATE edge_runtime_android)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)