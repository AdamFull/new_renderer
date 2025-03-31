
target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_application.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_application.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_headless_wIndow.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_headless_wIndow.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_input_events.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_native_surface.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_runtime.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_runtime.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_runtime_context.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/rt_window.h"
)

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/common/rt_glfw_native_surface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/common/rt_glfw_native_surface.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/common/rt_glfw_window.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/common/rt_glfw_window.h"

        "${CMAKE_CURRENT_SOURCE_DIR}/win64/rt_windows_native_surface.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/win64/rt_windows_runtime.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/win64/rt_windows_runtime.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/win64/rt_windows_runtime_context.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/win64/rt_windows_runtime_context.h"
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
elseif(ANDROID)
    target_sources(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/android/rt_android_common.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/android/rt_android_common.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/android/rt_android_native_surface.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/android/rt_android_native_surface.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/android/rt_android_runtime.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/android/rt_android_runtime.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/android/rt_android_runtime_context.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/android/rt_android_runtime_context.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/android/rt_android_window.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/android/rt_android_window.h"
    )
    target_link_libraries(${PROJECT_NAME} 
        PRIVATE
        log 
        android 
        game-activity::game-activity
    )
endif()