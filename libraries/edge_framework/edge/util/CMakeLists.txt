project(edge_utils)

#tracy
set(TRACY_ENABLE ON CACHE BOOL "Enable profiling" FORCE)
set(TRACY_ON_DEMAND ON CACHE BOOL "On-demand profiling" FORCE)
if(WIN32)
set(TRACY_CALLSTACK ON CACHE BOOL "Enfore callstack collection for tracy regions" FORCE)
endif()

find_package(Tracy CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# Enable tracy profiler
#add_definitions(-DTRACY_ENABLE)

file(GLOB EDGE_FRAMEWORK_UTILS_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${EDGE_FRAMEWORK_UTILS_SOURCES})

add_compile_definitions(
    #TRACY_ENABLE
    SPDLOG_USD_STD_FORMAT
)
    

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE Tracy::TracyClient
    PRIVATE spdlog::spdlog
)