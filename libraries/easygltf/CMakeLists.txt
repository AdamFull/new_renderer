cmake_minimum_required(VERSION 3.18.0)
project(easygltf)

find_package(glm CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)

file(GLOB_RECURSE EASYGLTF_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE EASYGLTF_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

 add_library(${PROJECT_NAME} STATIC ${EASYGLTF_SOURCES} ${EASYGLTF_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${PROJECT_NAME} 
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/"
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE 
    meshoptimizer::meshoptimizer
    glm::glm
)