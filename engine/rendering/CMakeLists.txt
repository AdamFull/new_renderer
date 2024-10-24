cmake_minimum_required(VERSION 3.18.0)
project(engine_rendering)

find_package(Ktx CONFIG REQUIRED)

#find_package(PkgConfig REQUIRED)  
#pkg_check_modules(LuaJIT REQUIRED IMPORTED_TARGET luajit)

find_package(sol2 CONFIG REQUIRED)

#add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/DebugDraw")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/gfx")

file(GLOB ENGINE_RENDERING_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ENGINE_RENDERING_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(RENDER_INTERFACE_SOURCES
    ${ENGINE_RENDERING_SOURCES}
    ${ENGINE_RENDERING_HEADERS}
)

add_library(${PROJECT_NAME} STATIC ${RENDER_INTERFACE_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/"
    PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/tools"
    PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/engine"
    PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/libraries/vma/include"
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE gfx
    PRIVATE KTX::ktx
)

target_compile_definitions(${PROJECT_NAME} 
	PUBLIC "VK_NO_PROTOTYPES"
)