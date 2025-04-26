include(GNUInstallDirs)

function(_slang_first_defined out)
    foreach(var IN LISTS ARGN)
        if(DEFINED "${var}")
            set("${out}" "${${var}}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
endfunction()

# ---- root hint ------------------------------------------------------------

_slang_first_defined(SLANG_ROOT_HINT
    Slang_ROOT
    ENV_Slang_ROOT
    SLANG_ROOT
    ENV_SLANG_ROOT
    SLANG_DIR)

if(NOT SLANG_ROOT_HINT AND DEFINED ENV{VULKAN_SDK})
    set(SLANG_ROOT_HINT "$ENV{VULKAN_SDK}")
endif()

# ---- locate header --------------------------------------------------------

set(_SLANG_HEADER_NAMES "slang.h")
find_path(Slang_INCLUDE_DIR NAMES ${_SLANG_HEADER_NAMES}
          HINTS "${SLANG_ROOT_HINT}"
          PATH_SUFFIXES "include" "include/slang" "Include" "Include/slang"
          DOC "Directory containing slang.h")

# ---- locate compiler executable ------------------------------------------

set(_SLANG_EXEC_NAMES "slangc" "slangc.exe")
find_program(Slang_COMPILER NAMES ${_SLANG_EXEC_NAMES}
             HINTS "${SLANG_ROOT_HINT}"
             PATH_SUFFIXES "bin" "Bin"
             DOC "Slang command-line compiler (slangc)")

# ---- locate library -------------------------------------------------------

set(_SLANG_LIB_NAMES "slang" "slang-static" "libslang")
# On MSVC prefer release/debug variants that match generator
if(MSVC AND CMAKE_CONFIGURATION_TYPES)
    list(INSERT _SLANG_LIB_NAMES 0 slang${CMAKE_${CMAKE_BUILD_TYPE}_POSTFIX})
endif()

find_library(Slang_LIBRARY NAMES ${_SLANG_LIB_NAMES}
             HINTS "${SLANG_ROOT_HINT}"
             PATH_SUFFIXES "lib" "Lib" "bin" "Bin"
             DOC "Slang compiler runtime/library")

# ---- imported target ------------------------------------------------------

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Slang REQUIRED_VARS
                                  Slang_INCLUDE_DIR
                                  Slang_COMPILER
                                  Slang_LIBRARY)

if(Slang_FOUND AND NOT TARGET Slang::slang)
    add_library(Slang::slang UNKNOWN IMPORTED)
    set_target_properties(Slang::slang PROPERTIES
        IMPORTED_LOCATION "${Slang_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Slang_INCLUDE_DIR}")
endif()

mark_as_advanced(Slang_INCLUDE_DIR Slang_LIBRARY Slang_COMPILER)