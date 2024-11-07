cmake_minimum_required(VERSION 3.18.0)
project(runtime LANGUAGES CXX)

find_package(glfw3 REQUIRED)

file(GLOB RUNTIME_SOURCES FILES *.cpp *.h)
file(GLOB UI_SOURCES FILES ui/*.cpp ui/*.h)
set(RUNTIME_SOURCES ${RUNTIME_SOURCES} ${UI_SOURCES})

if(WIN32)
    file(GLOB_RECURSE WINDOWS_RUNTIME_SOURCES FILES windows/*.cpp windows/*.h)
    set(RUNTIME_SOURCES ${RUNTIME_SOURCES} ${WINDOWS_RUNTIME_SOURCES})
endif()

add_library(${PROJECT_NAME} STATIC ${RUNTIME_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE glfw
)