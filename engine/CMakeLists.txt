cmake_minimum_required(VERSION 3.18.0)
project(application)

add_subdirectory(runtime)
#add_subdirectory(render)
add_subdirectory(rendering)
add_subdirectory(filesystem)
#add_subdirectory(scripting)
add_subdirectory(profiler)
add_subdirectory(world)
add_subdirectory(integrations)

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
	PUBLIC glm_module
	PUBLIC virtual_filesystem
	PRIVATE engine_profiler
	PRIVATE runtime
	PRIVATE engine_rendering
	#PRIVATE render_interface
	#PRIVATE script_engine
	PRIVATE engine_world
	PRIVATE imgui_integration
)

if(ANDROID)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_ANDROID_KHR PLATFORM_ANDROID)
elseif(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_WIN32_KHR PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_METAL_EXT PLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM_LINUX)

    # Choose WSI based on VKB_WSI_SELECTION
    if (VKE_WSI_SELECTION STREQUAL XCB OR VKE_WSI_SELECTION STREQUAL XLIB OR VKE_WSI_SELECTION STREQUAL WAYLAND)
        find_package(PkgConfig REQUIRED)
    endif()
    if (VKE_WSI_SELECTION STREQUAL XCB)
        pkg_check_modules(XCB xcb REQUIRED)
        if (XCB_FOUND)
            target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_XCB_KHR)
        endif()
    elseif (VKE_WSI_SELECTION STREQUAL XLIB)
        pkg_check_modules(X11 x11 REQUIRED)
        if (X11_FOUND)
            target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_XLIB_KHR)
        endif()
    elseif (VKE_WSI_SELECTION STREQUAL WAYLAND)
        pkg_check_modules(WAYLAND wayland-client REQUIRED)
        if (WAYLAND_FOUND)
            target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_WAYLAND_KHR)
        endif()
    elseif (VKE_WSI_SELECTION STREQUAL D2D)
        set(DIRECT_TO_DISPLAY TRUE)
        set(DIRECT_TO_DISPLAY TRUE PARENT_SCOPE)
        target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_DISPLAY_KHR PLATFORM_LINUX_D2D)
    else()
        message(FATAL_ERROR "Unknown WSI")
    endif()
endif()