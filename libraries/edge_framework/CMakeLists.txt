cmake_minimum_required(VERSION 3.22.1)

if (VCPKG_TARGET_ANDROID)
    include("cmake/vcpkg_android.cmake")
endif()

add_subdirectory(edge)