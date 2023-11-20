cmake_minimum_required(VERSION 3.18.0)
project(platform_specific)

find_package(glfw3 REQUIRED)

set(PLATFORM_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/PlatformSpecific.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/window/WindowCallback.cpp"
)
set(PLATFORM_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/window/WindowCallback.h"
)

if(WIN32)
set(PLATFORM_SOURCES 
    "${PLATFORM_SOURCES}" 
    "${CMAKE_CURRENT_SOURCE_DIR}/window/WindowWin32.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/window/WindowGLFW.cpp"
)
set(PLATFORM_HEADERS 
    "${PLATFORM_HEADERS}" 
    "${CMAKE_CURRENT_SOURCE_DIR}/window/WindowWin32.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/window/WindowGLFW.h"
)
endif()

add_library(${PROJECT_NAME} STATIC ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})

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
    PRIVATE glfw
)