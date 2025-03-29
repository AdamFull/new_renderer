cmake_minimum_required(VERSION 3.18.0)
project(engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(MAIN_PROJECT_DIRECTORY_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(OUT_BINARIES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(ENGINE_BINARIES_PATH ${OUT_BINARIES_PATH}/${CMAKE_BUILD_TYPE}/engine)
set(TOOLS_BINARIES_PATH ${OUT_BINARIES_PATH}/${CMAKE_BUILD_TYPE}/tools)

file(MAKE_DIRECTORY ${OUT_BINARIES_PATH})

#glslang
set(BUILD_EXTERNAL OFF CACHE BOOL "Build external dependencies in /External" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Builds glslangValidator and spirv-remap" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "Enables testing" FORCE)

#ktx
set(KTX_FEATURE_TESTS OFF CACHE BOOL "Create unit tests" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "Create KTX tools" FORCE)
set(KTX_FEATURE_VULKAN OFF CACHE BOOL "Enable Vulkan texture upload" FORCE)
set(KTX_FEATURE_GL_UPLOAD OFF CACHE BOOL "Enable OpenGL texture upload" FORCE)

#spirv-cross
#set(SPIRV_CROSS_SHARED OFF CACHE BOOL "Build the C API as a single shared library." FORCE)
set(SPIRV_CROSS_STATIC ON CACHE BOOL "Build the C and C++ API as static libraries." FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "Build the CLI binary. Requires SPIRV_CROSS_STATIC." FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Enable SPIRV-Cross tests." FORCE)
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "Enable util module support." FORCE)

#tracy
set(TRACY_ENABLE ON CACHE BOOL "Enable profiling" FORCE)
set(TRACY_ON_DEMAND ON CACHE BOOL "On-demand profiling" FORCE)
if(WIN32)
set(TRACY_CALLSTACK ON CACHE BOOL "Enfore callstack collection for tracy regions" FORCE)
endif()

if(WIN32)
add_compile_definitions("PLATFORM_WIN32")
endif()

if(MSVC)
    add_compile_options(/bigobj)
endif()

add_compile_definitions(
    IMGUI_DEFINE_MATH_OPERATORS
    IMGUI_USE_WCHAR32
    NOMINMAX
)

find_package(Taskflow CONFIG REQUIRED)
# target_link_libraries(main PRIVATE Taskflow::Taskflow)

# global includes
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}/exports"
    "${CMAKE_CURRENT_SOURCE_DIR}/libraries/utllib/other"
    "${CMAKE_CURRENT_SOURCE_DIR}/libraries/utllib/source"
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
)

add_subdirectory(libraries/utllib)
add_subdirectory(tools/scene_compiler)

add_subdirectory(engine)

add_executable(${PROJECT_NAME} main.cpp Application.cpp)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC utility_lib
	PRIVATE application
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${ENGINE_BINARIES_PATH}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${ENGINE_BINARIES_PATH}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${ENGINE_BINARIES_PATH}
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${ENGINE_BINARIES_PATH}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${ENGINE_BINARIES_PATH}
)