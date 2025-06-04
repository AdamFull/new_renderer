cmake_minimum_required(VERSION 3.18.0)
project(e2d_app)

file(GLOB E2D_APP_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC 
    ${E2D_APP_SOURCES}
)

if(MSVC)
	# mask out the min/max macros from minwindef.h
    add_definitions(-DNOMINMAX)
	target_compile_options(${PROJECT_NAME} PUBLIC /MP)
    add_compile_options(/wd4996)
else()
	target_compile_options(${PROJECT_NAME} PUBLIC -fexceptions)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_library(e2d_common INTERFACE)
target_link_libraries(e2d_common
    INTERFACE
    EnTT::EnTT 
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    edge_framework
    e2d_common 
)