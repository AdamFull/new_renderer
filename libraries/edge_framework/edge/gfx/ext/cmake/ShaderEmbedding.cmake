set(GLSL_FLAGS --target-env vulkan1.1 --glsl-version 460 "-P#extension GL_GOOGLE_include_directive : require")

# Ensure the Vulkan SDK environment is set up
if(NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK environment variable is not set. Please set up the Vulkan SDK.")
endif()

# Find glslc compiler from Vulkan SDK
find_program(GLSLANG_EXECUTABLE glslang HINTS "$ENV{VULKAN_SDK}/Bin" REQUIRED)

function(add_shader_compile_glsl SHADER_SOURCE)
    set(ARGS_OPTIONS STUB)
    set(ARGS_ONE EMBED_TARGET OUTPUT_FILE)
    set(ARGS_MULTI GLSLANG_FLAGS INCLUDE_DIRECTORIES DEFINES)

    cmake_parse_arguments(PARSE_ARGV 1 OPTION "${ARGS_OPTIONS}" "${ARGS_ONE}" "${ARGS_MULTI}")

    cmake_path(GET SHADER_SOURCE PARENT_PATH SHADER_REL_DIR)
    cmake_path(RELATIVE_PATH SHADER_REL_DIR)
    cmake_path(GET SHADER_SOURCE FILENAME SHADER_SOURCE)
    cmake_path(SET SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_REL_DIR})

    cmake_path(SET SHADER_SOURCE_FILE ${SHADER_SOURCE_DIR}/${SHADER_SOURCE})
    if(NOT EXISTS ${SHADER_SOURCE_FILE})
        message(FATAL_ERROR "Could not find shader source file ${SHADER_SOURCE_FILE}")
    endif()

    set(GLSLANG_INCLUDE_FLAGS "")
    foreach(inc_dir ${OPTION_INCLUDE_DIRECTORIES})
        list(APPEND GLSLANG_INCLUDE_FLAGS -I$<PATH:ABSOLUTE_PATH,${inc_dir},${CMAKE_CURRENT_SOURCE_DIR}>)
    endforeach()

    set(GLSLANG_DEFINE_FLAGS "")
    foreach(def ${OPTION_DEFINES})
        list(APPEND GLSLANG_DEFINE_FLAGS -D${def})
    endforeach()

    set(GLSLANG_FLAGS ${OPTION_GLSLANG_FLAGS} ${GLSLANG_INCLUDE_FLAGS} ${GLSLANG_DEFINE_FLAGS})

    set(SHADER_INC ${OPTION_EMBED_TARGET}.inc)
    set(SHADER_H ${OPTION_EMBED_TARGET}.h)
    set(SHADER_C ${OPTION_EMBED_TARGET}.c)
    cmake_path(SET SHADER_INC_FILE ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_INC})
    cmake_path(SET SHADER_H_FILE ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_H})
    cmake_path(SET SHADER_C_FILE ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_C})

    set(h_file_src "
      #pragma once
      #include <stddef.h>
      #include <stdint.h>

      #ifdef __cplusplus
      extern \"C\" {
      #endif

      extern const uint32_t ${OPTION_EMBED_TARGET}[];
      extern const size_t ${OPTION_EMBED_TARGET}_size;

      #ifdef __cplusplus
      }
      #endif")

    set(h_file)
    if(EXISTS ${SHADER_H_FILE})
      file(READ ${SHADER_H_FILE} h_file)
    endif()

    if(NOT h_file STREQUAL h_file_src)
      file(WRITE ${SHADER_H_FILE} "${h_file_src}")
    endif()

    set(c_file_src "
      #include \"${SHADER_H_FILE}\"

      const uint32_t ${OPTION_EMBED_TARGET}[] = {
        #include \"${SHADER_INC_FILE}\"
      };
      const size_t ${OPTION_EMBED_TARGET}_size = sizeof(${OPTION_EMBED_TARGET});")

    set(c_file)
    if(EXISTS ${SHADER_C_FILE})
      file(READ ${SHADER_C_FILE} c_file)
    endif()

    if(NOT c_file STREQUAL c_file_src)
      file(WRITE ${SHADER_C_FILE} "${c_file_src}")
    endif()

    add_custom_command(
      OUTPUT ${SHADER_INC_FILE}
      DEPENDS ${SHADER_SOURCE_FILE}
      COMMAND ${GLSLANG_EXECUTABLE} ${GLSLANG_FLAGS} ${SHADER_SOURCE_FILE} -x -o
              ${SHADER_INC_FILE} --depfile ${SHADER_INC_FILE}.d
      DEPFILE ${SHADER_INC_FILE}.d
      COMMAND_EXPAND_LISTS VERBATIM)
    set(SHADER_INC_TARGET ${OPTION_EMBED_TARGET}-Inc)
    add_custom_target(${SHADER_INC_TARGET} DEPENDS ${SHADER_INC_FILE})

    add_library(${OPTION_EMBED_TARGET} STATIC ${SHADER_H_FILE} ${SHADER_C_FILE})
    target_include_directories(${OPTION_EMBED_TARGET}
                               PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    add_dependencies(${OPTION_EMBED_TARGET} ${SHADER_INC_TARGET})

endfunction()