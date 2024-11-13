cmake_minimum_required(VERSION 3.18.0)
project(media_engine)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/audio")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/video")

file(GLOB MEDIA_ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} STATIC ${MEDIA_ENGINE_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/"
    PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/"
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE media_engine_audio
    PRIVATE media_engine_video
)