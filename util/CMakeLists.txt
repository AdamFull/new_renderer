cmake_minimum_required(VERSION 3.18.0)
project(utils LANGUAGES CXX)

find_package(spdlog CONFIG REQUIRED)

file(GLOB_RECURSE UTILS_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${UTILS_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE spdlog::spdlog
)