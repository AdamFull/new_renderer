#tracy
set(TRACY_ENABLE ON CACHE BOOL "Enable profiling" FORCE)
set(TRACY_ON_DEMAND ON CACHE BOOL "On-demand profiling" FORCE)
if(WIN32)
set(TRACY_CALLSTACK ON CACHE BOOL "Enfore callstack collection for tracy regions" FORCE)
endif()

target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/debugbreak.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lockfree_ring_buffer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Logger.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Result.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Serialization.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utl_base.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utl_enum.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utl_hash.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utl_memory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utl_memory.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/utl_profiler.h"
    #"${CMAKE_CURRENT_SOURCE_DIR}/utl_scheduler.cpp"
    #"${CMAKE_CURRENT_SOURCE_DIR}/utl_scheduler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utl_string.hpp"
    #"${CMAKE_CURRENT_SOURCE_DIR}/utl_thread_pool.cpp"
    #"${CMAKE_CURRENT_SOURCE_DIR}/utl_thread_pool.h"
)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC
    Tracy::TracyClient 
    nlohmann_json::nlohmann_json
)