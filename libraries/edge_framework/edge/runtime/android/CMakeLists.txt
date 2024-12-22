project(edge_runtime_android LANGUAGES CXX)

find_package(game-activity REQUIRED CONFIG)

file(GLOB_RECURSE EDGE_ANDROID_RUNTIME_SOURCES FILES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${EDGE_ANDROID_RUNTIME_SOURCES})

target_link_libraries(${PROJECT_NAME} 
	PRIVATE log android game-activity::game-activity
)