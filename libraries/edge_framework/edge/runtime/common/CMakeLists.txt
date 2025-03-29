project(edge_runtime_common LANGUAGES CXX)

file(GLOB_RECURSE EDGE_COMMON_RUNTIME_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${EDGE_COMMON_RUNTIME_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC glfw)