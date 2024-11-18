cmake_minimum_required(VERSION 3.18.0)
project(sfx)

add_subdirectory(OpenAl)
add_subdirectory(XAudio2)

file(GLOB SFX_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${SFX_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC "${MAIN_PROJECT_DIRECTORY_PATH}/engine"
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC sfx_openal
	PUBLIC sfx_xaudio2
)