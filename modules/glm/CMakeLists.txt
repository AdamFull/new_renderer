file(GLOB GLM_MODULES ${CMAKE_CURRENT_LIST_DIR}/*.cppm)

add_library(glm_module STATIC)
target_sources(glm_module PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${GLM_MODULES})
target_include_directories(glm_module PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(glm_module PROPERTIES LINKER_LANGUAGE CXX)