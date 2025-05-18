#tracy
set(TRACY_ENABLE ON CACHE BOOL "Enable profiling" FORCE)
set(TRACY_ON_DEMAND ON CACHE BOOL "On-demand profiling" FORCE)
if(WIN32)
set(TRACY_CALLSTACK ON CACHE BOOL "Enfore callstack collection for tracy regions" FORCE)
endif()

target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/utl_memory.cpp"
    #"${CMAKE_CURRENT_SOURCE_DIR}/utl_scheduler.cpp"
    #"${CMAKE_CURRENT_SOURCE_DIR}/utl_thread_pool.cpp"
)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC
    Tracy::TracyClient 
    nlohmann_json::nlohmann_json
)