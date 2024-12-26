cmake_minimum_required(VERSION 3.18.0)
project(EdgeHelloTriangle)

set(EDGE_HELLO_TRIANGLE_SOURCES main.cpp)

if(ANDROID)
    if(CMAKE_VS_NsightTegra_VERSION)
        list(APPEND EDGE_HELLO_TRIANGLE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/android/AndroidManifest.xml)
    endif()

    add_library(${PROJECT_NAME} SHARED ${EDGE_HELLO_TRIANGLE_SOURCES})
elseif(IOS)
	list(REMOVE_AT EDGE_HELLO_TRIANGLE_SOURCES 0)
	list(APPEND EDGE_HELLO_TRIANGLE_SOURCES
			${CMAKE_CURRENT_SOURCE_DIR}/ios/main.mm
			${CMAKE_CURRENT_SOURCE_DIR}/ios/AppDelegate.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios/AppDelegate.m
			${CMAKE_CURRENT_SOURCE_DIR}/ios/ViewController.h
			${CMAKE_CURRENT_SOURCE_DIR}/ios/ViewController.mm
	)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${EDGE_HELLO_TRIANGLE_SOURCES})
else()
    add_executable(${PROJECT_NAME} WIN32 ${EDGE_HELLO_TRIANGLE_SOURCES})
endif()

target_include_directories(${PROJECT_NAME} 
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../../"
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    edge_framework
)