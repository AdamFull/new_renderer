cmake_minimum_required(VERSION 3.22.1)
project(edge_shader_compiler)

find_package(argparse CONFIG REQUIRED)

set(VULKAN_SDK_ROOT $ENV{VULKAN_SDK})

find_path(SLANG_INCLUDE_DIR
  NAMES "slang.h"
  PATHS "${VULKAN_SDK_ROOT}"
  PATH_SUFFIXES "Include/slang" "include/slang"
)

find_library(SLANG_LIBRARY
  NAMES slang
  HINTS
    "${VULKAN_SDK_ROOT}/Lib"
    "${VULKAN_SDK_ROOT}/lib"
)

if(NOT SLANG_INCLUDE_DIR OR NOT SLANG_LIBRARY)
  message(FATAL_ERROR "Could not find Slang in Vulkan SDK or system paths.")
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${SLANG_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    spdlog
    argparse::argparse
    ${SLANG_LIBRARY}
)



# Lookup for slang
