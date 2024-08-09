file(GLOB IMGUI_MODULES ${CMAKE_CURRENT_LIST_DIR}/*.cppm)

add_library(imgui_module STATIC)
target_sources(imgui_module PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${IMGUI_MODULES})
target_include_directories(imgui_module PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(imgui_module PROPERTIES LINKER_LANGUAGE CXX)