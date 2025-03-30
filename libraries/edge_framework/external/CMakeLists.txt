cmake_minimum_required(VERSION 3.10)
project(external_libraries LANGUAGES C CXX)

# argparse
add_library(argparse INTERFACE)
set(ARGPARSE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/argparse/include)
target_sources(argparse INTERFACE ${ARGPARSE_INCLUDE_DIR}/argparse/argparse.hpp)
target_include_directories(argparse SYSTEM INTERFACE ${ARGPARSE_INCLUDE_DIR})
add_library(argparse::argparse ALIAS argparse)

# glm
add_library(glm INTERFACE)
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_sources(glm INTERFACE ${GLM_DIR}/glm/glm.hpp)
target_include_directories(glm SYSTEM INTERFACE ${GLM_DIR})

target_compile_definitions(glm INTERFACE
    GLM_FORCE_SWIZZLE
    GLM_FORCE_RADIANS
    GLM_FORCE_CTOR_INIT
    GLM_ENABLE_EXPERIMENTAL
)
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_definitions(glm INTERFACE GLM_FORCE_CXX20)
endif()

add_library(glm::glm ALIAS glm)

# refl-cpp
add_library(refl-cpp INTERFACE)
set(REFLCPP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/refl-cpp/include)
target_sources(refl-cpp INTERFACE ${REFLCPP_INCLUDE_DIR}/refl.hpp)
target_include_directories(refl-cpp SYSTEM INTERFACE ${REFLCPP_INCLUDE_DIR})

# nlohmann json
add_library(nlohmann_json INTERFACE)
set(NLOHMANN_JSON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/json/include)
target_sources(nlohmann_json INTERFACE 
    "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/adl_serializer.hpp"
    "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/byte_container_with_subtype.hpp"
    "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/json.hpp"
    "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/json_fwd.hpp"
    "${NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/ordered_map.hpp"
)
target_include_directories(nlohmann_json SYSTEM INTERFACE ${NLOHMANN_JSON_INCLUDE_DIR})
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)

# taskflow
add_library(taskflow INTERFACE)
set(TASKFLOW_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/taskflow)
target_sources(taskflow INTERFACE ${TASKFLOW_INCLUDE_DIR}/taskflow/taskflow.hpp)
target_include_directories(taskflow SYSTEM INTERFACE ${TASKFLOW_INCLUDE_DIR})

# tinygltf
add_library(tinygltf INTERFACE)
set(TINYGLTF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)
target_sources(tinygltf INTERFACE ${TINYGLTF_DIR}/tiny_gltf.h ${TINYGLTF_DIR}/json.hpp)
target_include_directories(tinygltf SYSTEM INTERFACE ${TINYGLTF_DIR})

# vulkan
add_library(vulkan INTERFACE)
set(VULKAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include)
target_sources(vulkan INTERFACE 
    ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h
    ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.hpp
)
target_include_directories(vulkan SYSTEM INTERFACE ${VULKAN_INCLUDE_DIR})

target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)

if(ANDROID)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
	target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
    # Choose WSI based on EDGE_VK_WSI_SELECTION
    if (VKB_WSI_SELECTION STREQUAL XCB OR EDGE_VK_WSI_SELECTION STREQUAL XLIB OR EDGE_VK_WSI_SELECTION STREQUAL WAYLAND)
        find_package(PkgConfig REQUIRED)
    endif()
    if (EDGE_VK_WSI_SELECTION STREQUAL XCB)
        pkg_check_modules(XCB xcb REQUIRED)
        if (XCB_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
        endif()
    elseif (EDGE_VK_WSI_SELECTION STREQUAL XLIB)
        pkg_check_modules(X11 x11 REQUIRED)
        if (X11_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XLIB_KHR)
        endif()
    elseif (EDGE_VK_WSI_SELECTION STREQUAL WAYLAND)
        pkg_check_modules(WAYLAND wayland-client REQUIRED)
        if (WAYLAND_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
        endif()
    elseif (EDGE_VK_WSI_SELECTION STREQUAL D2D)
        set(DIRECT_TO_DISPLAY TRUE)
        set(DIRECT_TO_DISPLAY TRUE PARENT_SCOPE)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_DISPLAY_KHR)
    else()
        message(FATAL_ERROR "Unknown WSI")
    endif()
endif()

add_library(Vulkan::Headers ALIAS vulkan)

# volk
set(VOLK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/volk")
set(VOLK_FILES
    "${VOLK_DIR}/volk.c"
    "${VOLK_DIR}/volk.h")

add_library(volk STATIC ${VOLK_FILES})
set_target_properties(volk PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(volk PUBLIC vulkan)

target_include_directories(volk SYSTEM PUBLIC ${VOLK_DIR})
if (EDGE_VK_WSI_SELECTION STREQUAL XCB)
    target_include_directories(volk SYSTEM PUBLIC ${XCB_INCLUDE_DIRS})
elseif (EDGE_VK_WSI_SELECTION STREQUAL XLIB)
    target_include_directories(volk SYSTEM PUBLIC ${X11_INCLUDE_DIRS})
elseif (EDGE_VK_WSI_SELECTION STREQUAL WAYLAND)
    target_include_directories(volk SYSTEM PUBLIC ${WAYLAND_INCLUDE_DIRS})
endif()

add_library(volk::volk ALIAS volk)

# vma
add_library(vma INTERFACE)
set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma/include)
target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
target_include_directories(vma SYSTEM INTERFACE ${VMA_DIR})
target_link_libraries(vma INTERFACE vulkan)

add_library(GPUOpen::VulkanMemoryAllocator ALIAS vma)

# zstd
set(ZSTD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ktx/external/basisu/zstd)
add_library(zstd STATIC ${ZSTD_DIR}/zstd.c)
target_include_directories(zstd SYSTEM INTERFACE ${ZSTD_DIR})

# zlib
#set(ZLIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
#
#set(ZLIB_SOURCES
#    "${ZLIB_SOURCE_DIR}/adler32.c"
#    "${ZLIB_SOURCE_DIR}/compress.c"
#    "${ZLIB_SOURCE_DIR}/crc32.c"
#    "${ZLIB_SOURCE_DIR}/crc32.h"
#    "${ZLIB_SOURCE_DIR}/deflate.c"
#    "${ZLIB_SOURCE_DIR}/deflate.h"
#    "${ZLIB_SOURCE_DIR}/gzclose.c"
#    "${ZLIB_SOURCE_DIR}/gzguts.h"
#    "${ZLIB_SOURCE_DIR}/gzlib.c"
#    "${ZLIB_SOURCE_DIR}/gzread.c"
#    "${ZLIB_SOURCE_DIR}/gzwrite.c"
#    "${ZLIB_SOURCE_DIR}/infback.c"
#    "${ZLIB_SOURCE_DIR}/inffast.c"
#    "${ZLIB_SOURCE_DIR}/inffast.h"
#    "${ZLIB_SOURCE_DIR}/inffixed.h"
#    "${ZLIB_SOURCE_DIR}/inflate.c"
#    "${ZLIB_SOURCE_DIR}/inflate.h"
#    "${ZLIB_SOURCE_DIR}/inftrees.c"
#    "${ZLIB_SOURCE_DIR}/inftrees.h"
#    "${ZLIB_SOURCE_DIR}/trees.c"
#    "${ZLIB_SOURCE_DIR}/trees.h"
#    "${ZLIB_SOURCE_DIR}/uncompr.c"
#    "${ZLIB_SOURCE_DIR}/zlib.h"
#    "${ZLIB_SOURCE_DIR}/zutil.c"
#    "${ZLIB_SOURCE_DIR}/zutil.h"
#)
#
#add_library(zlib STATIC ${ZLIB_SOURCES})
#target_include_directories(zlib SYSTEM PUBLIC ${ZLIB_SOURCE_DIR})
#
#if(WIN32)
#    target_compile_definitions(zlib PRIVATE _CRT_SECURE_NO_WARNINGS)
#endif()
#
#add_library(ZLIB::ZLIB ALIAS zlib)
#
## libzip
#set(LIBZIP_BUILD_TOOLS OFF CACHE BOOL "Disable libzip tools"   FORCE)
#set(LIBZIP_BUILD_EXAMPLES OFF CACHE BOOL "Disable libzip examples" FORCE)
#set(LIBZIP_BUILD_DOC OFF CACHE BOOL "Disable libzip docs" FORCE)
#add_subdirectory(libzip)

# libktx
set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ktx)

set(KTX_SOURCES
    ${KTX_DIR}/lib/checkheader.c
    ${KTX_DIR}/external/dfdutils/createdfd.c
    ${KTX_DIR}/external/dfdutils/colourspaces.c
    ${KTX_DIR}/external/dfdutils/dfd.h
    ${KTX_DIR}/external/dfdutils/dfd2vk.inl
    ${KTX_DIR}/external/dfdutils/interpretdfd.c
    ${KTX_DIR}/external/dfdutils/printdfd.c
    ${KTX_DIR}/external/dfdutils/queries.c
    ${KTX_DIR}/external/dfdutils/vk2dfd.c
    ${KTX_DIR}/external/etcdec/etcdec.cxx
    ${KTX_DIR}/lib/etcunpack.cxx
    ${KTX_DIR}/lib/filestream.c
    ${KTX_DIR}/lib/filestream.h
    ${KTX_DIR}/lib/formatsize.h
    ${KTX_DIR}/lib/hashlist.c
    ${KTX_DIR}/lib/info.c
    ${KTX_DIR}/lib/ktxint.h
    ${KTX_DIR}/lib/memstream.c
    ${KTX_DIR}/lib/memstream.h
    ${KTX_DIR}/lib/strings.c
    ${KTX_DIR}/lib/swap.c
    ${KTX_DIR}/lib/uthash.h
    ${KTX_DIR}/lib/texture.c
    ${KTX_DIR}/lib/texture.h
    ${KTX_DIR}/utils/unused.h

     # Basis Universal
    ${KTX_DIR}/lib/basis_sgd.h
    ${KTX_DIR}/lib/basis_transcode.cpp
    ${KTX_DIR}/lib/miniz_wrapper.cpp
    ${KTX_DIR}/external/basisu/transcoder/basisu_containers.h
    ${KTX_DIR}/external/basisu/transcoder/basisu_containers_impl.h
    ${KTX_DIR}/external/basisu/transcoder/basisu_file_headers.h
    ${KTX_DIR}/external/basisu/transcoder/basisu_transcoder_internal.h
    ${KTX_DIR}/external/basisu/transcoder/basisu_transcoder_uastc.h
    ${KTX_DIR}/external/basisu/transcoder/basisu_transcoder.cpp
    ${KTX_DIR}/external/basisu/transcoder/basisu_transcoder.h
    ${KTX_DIR}/external/basisu/transcoder/basisu.h

    # KT1
    ${KTX_DIR}/lib/texture1.c
    ${KTX_DIR}/lib/texture1.h

    # KTX2
    ${KTX_DIR}/lib/texture2.c
    ${KTX_DIR}/lib/texture2.h

    # Vulkan support
    ${KTX_DIR}/lib/vk_format.h
    ${KTX_DIR}/lib/vkformat_check.c
    ${KTX_DIR}/lib/vkformat_check_variant.c
    ${KTX_DIR}/lib/vkformat_enum.h
    ${KTX_DIR}/lib/vkformat_str.c
    ${KTX_DIR}/lib/vkformat_typesize.c
    ${KTX_DIR}/lib/vk_funcs.c
    ${KTX_DIR}/lib/vk_funcs.h
    ${KTX_DIR}/lib/vkloader.c
)

set(KTX_INCLUDE_DIRS
    ${KTX_DIR}/include
    ${KTX_DIR}/lib
    ${KTX_DIR}/utils
    ${KTX_DIR}/external    
    ${KTX_DIR}/external/basisu/zstd
    ${KTX_DIR}/external/basisu/transcoder
    ${KTX_DIR}/other_include
)

add_library(ktx STATIC ${KTX_SOURCES})

target_compile_definitions(ktx PUBLIC LIBKTX)
if (WIN32)
target_compile_definitions(ktx PUBLIC "KTX_API=__declspec(dllexport)")
endif()
target_compile_definitions(ktx PUBLIC KTX_FEATURE_WRITE=0)
target_compile_definitions(ktx PUBLIC BASISD_SUPPORT_KTX2_ZSTD=0)
target_compile_definitions(ktx PUBLIC BASISU_NO_ITERATOR_DEBUG_LEVEL)

target_include_directories(ktx SYSTEM PUBLIC ${KTX_INCLUDE_DIRS})

target_link_libraries(ktx PUBLIC 
    zstd
    vulkan
)

add_library(KTX::ktx ALIAS ktx)

# imgui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_FILES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp"
    "${IMGUI_DIR}/misc/freetype/imgui_freetype.cpp"
    "${IMGUI_DIR}/imconfig.h"
    "${IMGUI_DIR}/imgui.h"
    "${IMGUI_DIR}/imgui_internal.h"
    "${IMGUI_DIR}/imstb_rectpack.h"
    "${IMGUI_DIR}/imstb_textedit.h"
    "${IMGUI_DIR}/imstb_truetype.h"
    "${IMGUI_DIR}/misc/cpp/imgui_stdlib.h"
    "${IMGUI_DIR}/misc/freetype/imgui_freetype.h"
)

add_library(imgui STATIC ${IMGUI_FILES})
target_include_directories(imgui SYSTEM PUBLIC ${IMGUI_DIR})

# glslang
option(ENABLE_SPVREMAPPER OFF)
option(ENABLE_GLSLANG_BINARIES OFF)
set(ENABLE_HLSL ON)
option(ENABLE_OPT OFF)
option(BUILD_TESTING OFF)
option(BUILD_EXTERNAL OFF)
if(IOS)
    option(ENABLE_PCH OFF)
endif ()

add_subdirectory(glslang)

if (NOT TARGET glslang-default-resource-limits)
    add_library(glslang-default-resource-limits
                glslang/StandAlone/ResourceLimits.cpp)

    target_include_directories(glslang-default-resource-limits
        SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glslang)
endif()

if(NOT MSVC)
    target_compile_options(glslang PRIVATE
        "-Wno-logical-op-parentheses"
        "-Wno-unused-parameter")

    target_compile_options(SPIRV PRIVATE
        "-Wno-logical-op-parentheses"
        "-Wno-unused-parameter")
endif()

# spirv-cross
#set(SPIRV_CROSS_SHARED OFF CACHE BOOL "Build the C API as a single shared library." FORCE)
set(SPIRV_CROSS_STATIC ON CACHE BOOL "Build the C and C++ API as static libraries." FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "Build the CLI binary. Requires SPIRV_CROSS_STATIC." FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Enable SPIRV-Cross tests." FORCE)
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "Enable util module support." FORCE)
add_subdirectory(spirv-cross)
#target_include_directories(spirv-cross-glsl SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross")

# stb
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

if(NOT ANDROID AND NOT IOS)
    if (NOT DIRECT_TO_DISPLAY)
        # GLFW
        option(GLFW_BUILD_DOCS OFF)
        option(GLFW_BUILD_TESTS OFF)
        option(GLFW_BUILD_EXAMPLES OFF)
        option(GLFW_BUILD_TESTS OFF)
        option(GLFW_INSTALL OFF)
        option(BUILD_SHARED_LIBS ON)
        if (EDGE_VK_WSI_SELECTION STREQUAL WAYLAND)
            set(GLFW_USE_WAYLAND ON)
        endif()

        add_subdirectory(glfw)
    endif()
endif()


# spdlog
set(SPDLOG_USE_STD_FORMAT ON)
add_subdirectory(spdlog)
target_include_directories(spdlog SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include)

# entt
add_library(entt INTERFACE)
set(ENTT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/entt/src)
target_sources(entt INTERFACE 
    "${ENTT_INCLUDE_DIR}/entt/entt.hpp"
    "${ENTT_INCLUDE_DIR}/entt/fwd.hpp"
)
target_include_directories(entt SYSTEM INTERFACE ${ENTT_INCLUDE_DIR})
add_library(EnTT::EnTT ALIAS entt)

add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME}
    INTERFACE
    glm::glm
    refl-cpp
    tinygltf
    spdlog::spdlog
    stb
    KTX::ktx
    glslang::glslang
	glslang::SPIRV

    spirv-cross-core
	spirv-cross-glsl

    nlohmann_json::nlohmann_json
    taskflow
    #vulkan
    #volk
    #vma
    #ktx
)