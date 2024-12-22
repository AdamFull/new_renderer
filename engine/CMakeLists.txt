cmake_minimum_required(VERSION 3.18.0)
project(application)

add_subdirectory(media)
add_subdirectory(rendering)
#add_subdirectory(scripting)
add_subdirectory(world)
add_subdirectory(ui)

file(GLOB ENGINE_APP_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${ENGINE_APP_SOURCES})

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

target_link_libraries(${PROJECT_NAME}
    PUBLIC edge_framework engine_rendering engine_ui media_engine engine_world
)